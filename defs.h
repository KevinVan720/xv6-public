struct buf;
struct context;
struct file;
struct inode;
struct pipe;
struct proc;
struct rtcdate;
struct spinlock;
struct sleeplock;
struct stat;
struct superblock;

struct RGB;
struct RGBA;
struct message;

//window_manager.c
void wmInit(void);
void wmHandleMessage(struct message *);

//msg.c
int handleMessage(struct message *);

//gui.c
extern int screen_size;
extern ushort SCREEN_WIDTH;
extern ushort SCREEN_HEIGHT;
extern struct RGB *screen;
extern struct RGB *screen_buf;
void initGUI(void);
int drawCharacter(struct RGB *, int, int, char, struct RGBA);
int drawIcon(struct RGB *buf, int x, int y, int icon, struct RGBA color);
void drawString(struct RGB *, int, int, char *, struct RGBA);
void drawStringWithMaxWidth(struct RGB *, int, int, int, char *, struct RGBA);
void drawMouse(struct RGB *, int, int, int);
void clearMouse(struct RGB *, struct RGB *, int, int);
void drawRect(struct RGB *, int, int, int, int, struct RGBA);
void clearRect(struct RGB *, struct RGB *, int, int, int, int);
void drawRectByCoord(struct RGB *, int, int, int, int, struct RGBA);
void clearRectByCoord(struct RGB *, struct RGB *, int, int, int, int);
void draw24Image(struct RGB *, struct RGB *, int, int, int, int, int, int);
void draw24ImagePart(struct RGB *, struct RGB *, int, int, int, int, int, int, int, int);
void drawImage(struct RGB *, struct RGBA *, int, int, int, int, int, int);
void drawRectBound(struct RGB *, int, int, int, int, struct RGBA, int, int);
void drawRectBorder(struct RGB *buf, struct RGB color, int x, int y, int width, int height);

// bio.c
void binit(void);
struct buf *bread(uint, uint);
void brelse(struct buf *);
void bwrite(struct buf *);

// console.c
void consoleinit(void);
void cprintf(char *, ...);
void consoleintr(int (*)(void));
void panic(char *) __attribute__((noreturn));

// exec.c
int exec(char *, char **);

// file.c
struct file *filealloc(void);
void fileclose(struct file *);
struct file *filedup(struct file *);
void fileinit(void);
int fileread(struct file *, char *, int n);
int filestat(struct file *, struct stat *);
int filewrite(struct file *, char *, int n);

// fs.c
void readsb(int dev, struct superblock *sb);
int dirlink(struct inode *, char *, uint);
struct inode *dirlookup(struct inode *, char *, uint *);
struct inode *ialloc(uint, short);
struct inode *idup(struct inode *);
void iinit(int dev);
void ilock(struct inode *);
void iput(struct inode *);
void iunlock(struct inode *);
void iunlockput(struct inode *);
void iupdate(struct inode *);
int namecmp(const char *, const char *);
struct inode *namei(char *);
struct inode *nameiparent(char *, char *);
int readi(struct inode *, char *, uint, uint);
void stati(struct inode *, struct stat *);
int writei(struct inode *, char *, uint, uint);

// ide.c
void ideinit(void);
void ideintr(void);
void iderw(struct buf *);

// ioapic.c
void ioapicenable(int irq, int cpu);
extern uchar ioapicid;
void ioapicinit(void);

// kalloc.c
char *kalloc(void);
void kfree(char *);
void kinit1(void *, void *);
void kinit2(void *, void *);

// kbd.c
void kbdintr(void);

// mouse.c
void mouseinit(void);
void mouseintr(uint);

// lapic.c
void cmostime(struct rtcdate *r);
int lapicid(void);
extern volatile uint *lapic;
void lapiceoi(void);
void lapicinit(void);
void lapicstartap(uchar, uint);
void microdelay(int);

// log.c
void initlog(int dev);
void log_write(struct buf *);
void begin_op();
void end_op();

// mp.c
extern int ismp;
void mpinit(void);

// picirq.c
void picenable(int);
void picinit(void);

// pipe.c
int pipealloc(struct file **, struct file **);
void pipeclose(struct pipe *, int);
int piperead(struct pipe *, char *, int);
int pipewrite(struct pipe *, char *, int);

//PAGEBREAK: 16
// proc.c
int cpuid(void);
void exit(void);
int fork(void);
int growproc(int);
int kill(int);
struct cpu *mycpu(void);
struct proc *myproc();
void pinit(void);
void procdump(void);
void scheduler(void) __attribute__((noreturn));
void sched(void);
void setproc(struct proc *);
void sleep(void *, struct spinlock *);
void userinit(void);
int wait(void);
void wakeup(void *);
void yield(void);

// swtch.S
void swtch(struct context **, struct context *);

// spinlock.c
void acquire(struct spinlock *);
void getcallerpcs(void *, uint *);
int holding(struct spinlock *);
void initlock(struct spinlock *, char *);
void release(struct spinlock *);
void pushcli(void);
void popcli(void);

// sleeplock.c
void acquiresleep(struct sleeplock *);
void releasesleep(struct sleeplock *);
int holdingsleep(struct sleeplock *);
void initsleeplock(struct sleeplock *, char *);

// string.c
int memcmp(const void *, const void *, uint);
void *memmove(void *, const void *, uint);
void *memset(void *, int, uint);
char *safestrcpy(char *, const char *, int);
int strlen(const char *);
int strncmp(const char *, const char *, uint);
char *strncpy(char *, const char *, int);

// syscall.c
int argint(int, int *);
int argptr(int, char **, int);
int argstr(int, char **);
int fetchint(uint, int *);
int fetchstr(uint, char **);
void syscall(void);

// timer.c
void timerinit(void);

// trap.c
void idtinit(void);
extern uint ticks;
void tvinit(void);
extern struct spinlock tickslock;

// uart.c
void uartinit(void);
void uartintr(void);
void uartputc(int);

// vm.c
void seginit(void);
void kvmalloc(void);
pde_t *setupkvm(void);
char *uva2ka(pde_t *, char *);
int allocuvm(pde_t *, uint, uint);
int deallocuvm(pde_t *, uint, uint);
void freevm(pde_t *);
void inituvm(pde_t *, char *, uint);
int loaduvm(pde_t *, char *, struct inode *, uint, uint);
pde_t *copyuvm(pde_t *, uint);
void switchuvm(struct proc *);
void switchkvm(void);
int copyout(pde_t *, uint, void *, uint);
void clearpteu(pde_t *pgdir, char *uva);

// number of elements in fixed-size array
#define NELEM(x) (sizeof(x) / sizeof((x)[0]))
