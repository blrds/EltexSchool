#include<stdio.h>
#include<unistd.h>

int main(){
	__pid_t pid=getpid();
	__pid_t ppid=getppid();
	__uid_t uid=getuid();
	__uid_t euid=geteuid();
	__gid_t gid=getgid();
	__pid_t sid=getsid(pid);
	printf("PID:%d, PPID:%d\n", pid, ppid);
	printf("UID:%d, EUID:%d\n", uid, euid);
	printf("GID:%d, SID:%d\n", gid, sid);
	return 1;
}
