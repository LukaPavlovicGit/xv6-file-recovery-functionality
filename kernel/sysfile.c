//
// File-system system calls.
// Mostly argument checking, since we don't trust
// user code, and calls into file.c and fs.c.
//

#include "types.h"
#include "defs.h"
#include "param.h"
#include "stat.h"
#include "mmu.h"
#include "proc.h"
#include "fs.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "file.h"
#include "fcntl.h"

// Fetch the nth word-sized system call argument as a file descriptor
// and return both the descriptor and the corresponding struct file.
static int
argfd(int n, int *pfd, struct file **pf)
{
	int fd;
	struct file *f;

	if(argint(n, &fd) < 0)
		return -1;
	if(fd < 0 || fd >= NOFILE || (f=myproc()->ofile[fd]) == 0)
		return -1;
	if(pfd)
		*pfd = fd;
	if(pf)
		*pf = f;
	return 0;
}

// Allocate a file descriptor for the given file.
// Takes over file reference from caller on success.
static int
fdalloc(struct file *f)
{
	int fd;
	struct proc *curproc = myproc();

	for(fd = 0; fd < NOFILE; fd++){
		if(curproc->ofile[fd] == 0){
			curproc->ofile[fd] = f;
			return fd;
		}
	}
	return -1;
}

int
sys_dup(void)
{
	struct file *f;
	int fd;

	if(argfd(0, 0, &f) < 0)
		return -1;
	if((fd=fdalloc(f)) < 0)
		return -1;
	filedup(f);
	return fd;
}

int
sys_read(void) // f,n,p su parametri sistemstom poziva od strane korisnickom programa ( read(fd,buf,9) )
{
	struct file *f;
	int n;
	char *p;


	if(argfd(0, 0, &f) < 0 || argint(2, &n) < 0 || argptr(1, &p, n) < 0)
		return -1;
	return fileread(f, p, n); // file.c
}

int
sys_write(void)
{
	struct file *f;
	int n;
	char *p;

	if(argfd(0, 0, &f) < 0 || argint(2, &n) < 0 || argptr(1, &p, n) < 0)		
		return -1;
	
	return filewrite(f, p, n); // file.c
}

int
sys_close(void)
{
	int fd;
	struct file *f;

	if(argfd(0, &fd, &f) < 0)
		return -1;
	myproc()->ofile[fd] = 0;
	fileclose(f);
	return 0;
}

int
sys_fstat(void)
{
	struct file *f;
	struct stat *st;

	if(argfd(0, 0, &f) < 0 || argptr(1, (void*)&st, sizeof(*st)) < 0)
		return -1;
	return filestat(f, st);
}

// Create the path new as a link to the same inode as old.
int
sys_link(void)
{
	char name[DIRSIZ], *new, *old;
	struct inode *dp, *ip;

	if(argstr(0, &old) < 0 || argstr(1, &new) < 0)
		return -1;

	begin_op();
	if((ip = namei(old)) == 0){
		end_op();
		return -1;
	}

	ilock(ip);
	if(ip->type == T_DIR){
		iunlockput(ip);
		end_op();
		return -1;
	}

	ip->nlink++;
	iupdate(ip);
	iunlock(ip);

	if((dp = nameiparent(new, name)) == 0)
		goto bad;
	ilock(dp);
	if(dp->dev != ip->dev || dirlink(dp, name, ip->inum) < 0){
		iunlockput(dp);
		goto bad;
	}
	iunlockput(dp);
	iput(ip);

	end_op();

	return 0;

bad:
	ilock(ip);
	ip->nlink--;
	iupdate(ip);
	iunlockput(ip);
	end_op();
	return -1;
}

// Is the directory dp empty except for "." and ".." ?
static int
isdirempty(struct inode *dp)
{
	int off;
	struct dirent de;

	for(off=2*sizeof(de); off<dp->size; off+=sizeof(de)){
		if(readi(dp, (char*)&de, off, sizeof(de)) != sizeof(de))
			panic("isdirempty: readi");
		if(de.inum != 0)
			return 0;
	}
	return 1;
}

int
sys_unlink(void)
{
	struct inode *ip, *dp;
	struct dirent de;
	char name[DIRSIZ], *path;
	uint off;

	if(argstr(0, &path) < 0)
		return -1;

	begin_op(); // POCETAK TRANSANKCIJE
	if((dp = nameiparent(path, name)) == 0){ // fs.c ; dp je parent od ip
		end_op();
		return -1;
	}

	ilock(dp);// ZAKLJUCAVANJE PRISTUPA dp INOD-U ; fs.c	

	// Cannot unlink "." or "..".
	if(namecmp(name, ".") == 0 || namecmp(name, "..") == 0)
		goto bad;

	if((ip = dirlookup(dp, name, &off)) == 0) // fs.c
		goto bad;
	ilock(ip); // ZAKLJUCAVANJE PRISTUPA dp INOD-U ; fs.c	

	if(ip->nlink < 1)
		panic("unlink: nlink < 1");
	if(ip->type == T_DIR && !isdirempty(ip)){
		iunlockput(ip);
		goto bad;
	}

	//memset(&de, 0, sizeof(de));

	strncpy(de.name, name, DIRSIZ); // dirent struktura ostaje ista, sem de.del koje postaje 1
	de.inum=ip->inum;
	de.del=1;
	
	if(writei(dp, (char*)&de, off, sizeof(de)) != sizeof(de)) // fs.c
		panic("unlink: writei");
	
	// dp je parent od ip
	if(ip->type == T_DIR){
		dp->nlink--;
		iupdate(dp);//@ fs.c
	}

	iunlockput(dp); // OTKLJUCAVANJE PROSTUPA dp INOD-U

	ip->nlink--;
	iupdate(ip);
	iunlockput(ip); // OTKLJUCAVANJE PROSTUPA IP INOD-U

	end_op(); // KRAJ TRANSAKCIJE

	return 0;

bad:
	iunlockput(dp);
	end_op();
	return -1;
}

static struct inode*
create(char *path, short type, short major, short minor)
{	
	struct inode *ip, *dp;
	char name[DIRSIZ];
	
	if((dp = nameiparent(path, name)) == 0) // fs.c
		return 0;	
	
	ilock(dp);
		
	if((ip = dirlookup(dp, name, 0)) != 0){ // fs.c
		iunlockput(dp);
		ilock(ip);
		if(type == T_FILE && ip->type == T_FILE)
			return ip;
		iunlockput(ip);
		return 0;
	}
	
	if((ip = ialloc(dp->dev, type)) == 0) // fs.c
		panic("create: ialloc");
	
	ilock(ip);
	ip->major = major;
	ip->minor = minor;
	ip->nlink = 1;
	iupdate(ip);

	if(type == T_DIR){  // Create . and .. entries.
		dp->nlink++;  // for ".."
		iupdate(dp);
		// No ip->nlink++ for ".": avoid cyclic ref count.
		if(dirlink(ip, ".", ip->inum) < 0 || dirlink(ip, "..", dp->inum) < 0) // fs.c
			panic("create dots");
	}

	if(dirlink(dp, name, ip->inum) < 0) // fs.c
		panic("create: dirlink");

	iunlockput(dp);

	return ip;
}
/*
path - putanja fajla koji otvaramo
omode - mode u kom radimo sa fajlom

path i omode se nalaze na korisnickom steku kom pristupamo uz pomoc trapframe-a

# prvi arg oznacava koji po redu argument zelimo iz korisnickog programa, u drugi arg se ucitava vrednost tog argumenta
# pogledaj syscall.c
int argstr(int, char**)
int argint(int, int*)
*/
int
sys_open(void)
{
	char *path;
	int fd, omode;
	struct file *f;
	struct inode *ip;
	if(argstr(0, &path) < 0 || argint(1, &omode) < 0)	
		return -1;

	begin_op();
	if(omode & O_CREATE){ // ako je setovan create flag pravi se datoteka u fajl sistemu
		ip = create(path, T_FILE, 0, 0);
		if(ip == 0){
			end_op();
			return -1;
		}
	} else {
		if((ip = namei(path)) == 0){ // pronalazi inode u fajl sistemu na datoj putanji (vraca inode koji se nalazi na kraju path-a)										
			end_op();
			return -1;
		}
		ilock(ip);
		if(ip->type == T_DIR && omode != O_RDONLY){ // ako je direktorijum, onda on moze samo da se cita			
			iunlockput(ip);
			end_op();
			return -1;
		}
	}

	if((f = filealloc()) == 0 || (fd = fdalloc(f)) < 0){ // filealloc() se nalazi u file.c, fdalloc(f) u trenutnom fajlu
		if(f)
			fileclose(f);
		iunlockput(ip);
		end_op();
		return -1;
	}
	iunlock(ip);
	end_op();

	f->type = FD_INODE;
	f->ip = ip;
	f->off = 0;
	f->readable = !(omode & O_WRONLY);
	f->writable = (omode & O_WRONLY) || (omode & O_RDWR);
	return fd; // fd je redni broj fajla koji se nalazi u nizu otvorenih fajlova (ofile[]) trenutnog procesa
}

int
sys_mkdir(void)
{
	char *path;
	struct inode *ip;
	begin_op();
	if(argstr(0, &path) < 0 || (ip = create(path, T_DIR, 0, 0)) == 0){		
		end_op();
		return -1;
	}
	iunlockput(ip);
	end_op();
	return 0;
}

int
sys_mknod(void)
{
	struct inode *ip;
	char *path;
	int major, minor;

	begin_op();
	if((argstr(0, &path)) < 0 ||
			argint(1, &major) < 0 ||
			argint(2, &minor) < 0 ||
			(ip = create(path, T_DEV, major, minor)) == 0){
		end_op();
		return -1;
	}
	iunlockput(ip);
	end_op();
	return 0;
}

int
sys_chdir(void)
{
	char *path;
	struct inode *ip;
	struct proc *curproc = myproc();

	begin_op();
	if(argstr(0, &path) < 0 || (ip = namei(path)) == 0){
		end_op();
		return -1;
	}
	ilock(ip);
	if(ip->type != T_DIR){
		iunlockput(ip);
		end_op();
		return -1;
	}
	iunlock(ip);
	iput(curproc->cwd);
	end_op();
	curproc->cwd = ip;
	return 0;
}

int
sys_exec(void)
{
	char *path, *argv[MAXARG];
	int i;
	uint uargv, uarg;

	if(argstr(0, &path) < 0 || argint(1, (int*)&uargv) < 0){
		return -1;
	}
	memset(argv, 0, sizeof(argv));
	for(i=0;; i++){
		if(i >= NELEM(argv))
			return -1;
		if(fetchint(uargv+4*i, (int*)&uarg) < 0)
			return -1;
		if(uarg == 0){
			argv[i] = 0;
			break;
		}
		if(fetchstr(uarg, &argv[i]) < 0)
			return -1;
	}
	return exec(path, argv);
}

int
sys_pipe(void)
{
	int *fd;
	struct file *rf, *wf;
	int fd0, fd1;

	if(argptr(0, (void*)&fd, 2*sizeof(fd[0])) < 0)
		return -1;
	if(pipealloc(&rf, &wf) < 0)
		return -1;
	fd0 = -1;
	if((fd0 = fdalloc(rf)) < 0 || (fd1 = fdalloc(wf)) < 0){
		if(fd0 >= 0)
			myproc()->ofile[fd0] = 0;
		fileclose(rf);
		fileclose(wf);
		return -1;
	}
	fd[0] = fd0;
	fd[1] = fd1;
	return 0;
}


int
sys_lsdel(void)
{	
	int fd; // file descriptor 
	struct dirent de;
	struct stat st;
	struct file *f;
	struct inode *ip;
	char *path, *result;
	
	if((argstr(0, &path)) < 0 || (argstr(1, &result)) < 0)
		return -1;

	
	begin_op();
	if((ip = namei(path)) == 0){ // vraca inode koji se nalazi na kraju path-a	
			end_op();
			return -1;
	}
	ilock(ip);
	if((f = filealloc()) == 0 || (fd = fdalloc(f)) < 0){ // filealloc() se nalazi u file.c, fdalloc(f) u trenutnom fajlu
		if(f)
			fileclose(f);
		iunlockput(ip);
		end_op();
		return -1;
	}
	iunlock(ip);
	end_op();
	
	f->type = FD_INODE;
	f->ip = ip;
	f->off = 0;
	f->readable = 1;
	f->writable = 0;

	int red=0, i=0, j=0; 								
	while(fileread(f, &de, sizeof(de)) == sizeof(de)){
		if(de.del == 1){
			char *name;
			strncpy(name, de.name, DIRSIZ);
			while(j<strlen(name)){
				result[i+j]=name[j];		// i-broj reda ; j-offset u i-tom redu
				j++;			
			}
			result[i+j]=0;
			if(red==64) 
				break;
			i = (++red)*(DIRSIZ+1);			
			j=0;		
		}			
	}
	
	return red;
}

int
sys_rec(void){
	struct inode *ip, *dp;
	struct dirent de;
	char name[DIRSIZ], *path;
	int bn;
	uint off;	
	
	argstr(0, &path);
	begin_op();
	
	// da li je putanja do roditeljskog direktorijuma ispravna ?
	if((dp = nameiparent(path, name)) == 0) {// fs.c 
		end_op();	
		return -1;
	}
	ilock(dp);
	// da li se obrisan fajl nalazi u direktorijumu ?
	if((ip = dirdeletedlookup(dp, name, &off)) == 0){ // fs.c
		iunlockput(dp);
		end_op();
		return -2;
	}
	// da li je inode iskoriscen za nesto drugo ? (UJEDNO I ZAKLJUCAVA INODE)
	if(checkinode(ip) == 0){ // fs.c
		iunlockput(dp);
		iunlock(ip);
		end_op();	
		return -3;
	}
	// da li je neki data-blok inode-a iskoriscen za nesto drugo ?
	bn = ((ip->size-1)/512 + 1);
	if((checkibloks(ip,bn)) == 0){ // fs.c
		
		// set unrecoverable (datoteka vise ne moze da se oporavi ; naredni put kada se pozove rec za ovu datoteku vratice -1)
		memset(&de, 0, sizeof(de));
		if(writei(dp, (char*)&de, off, sizeof(de)) != sizeof(de)) // fs.c
			panic("unlink: writei");

		iunlockput(dp);
		iunlock(ip);
		end_op();
		return -4;
	}
	// da li u direktorujumu postoji datoteka koja ima isto ime kao datoteka koju oporavljamo?
	if(dirlookup(dp,name,0) > 0){
		int k = strlen(name)-1;
		char c = (name[k]+1 > 122) ? 'a' : name[k] + 1;
		
		if(k+1 < DIRSIZ)
			name[k + 1] = c; // dodajemo karakter
		else
			name[k] = c; // menjamo ime poslednjem karakteru
	}

	recovery(ip,bn);
	
	strncpy(de.name, name, DIRSIZ); 
	de.inum=ip->inum;
	de.del=0;
	
	if(writei(dp, (char*)&de, off, sizeof(de)) != sizeof(de)) // fs.c ; upisujemo izmene na disk
		panic("sys_rec: writei");
		
	ip->nlink++;
	iupdate(ip); // upisujemo izmene na disk
	
	iunlockput(dp);
	iunlockput(ip);
	end_op();
	
	return 0;
}

int 
sys_provera(void)
{
	struct inode *ip, *dp;
	char name[DIRSIZ], *path;
	int bn;
	
	argstr(0, &path);
	
	begin_op();
	// da li je putanja do roditeljskog direktorijuma ispravna ?
	if((dp = nameiparent(path, name)) == 0) {// fs.c 
		end_op();	
		return -1;
	}
	ilock(dp);
	if((ip = dirlookup(dp, name, 0)) == 0){ // fs.c
		iunlockput(dp);		
		end_op();			
		return 0;
	}
	ilock(ip);
	
	bn = ((ip->size-1)/512 + 1);
	struct buf *bp;	
	int block, blockoff, byte, byteoff, m;	
	int k = (bn > NDIRECT) ? NDIRECT : bn;
	
	for(int i=0;i<k;++i){
		cprintf("ip->addrs[i] : %d\n",ip->addrs[i]);		

	}

	iunlockput(dp);	
	iunlockput(ip);
	end_op();
	return 1;

}
