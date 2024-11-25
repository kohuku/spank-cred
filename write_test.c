#include <slurm/spank.h>
#include <stdio.h>
#include <unistd.h>

SPANK_PLUGIN(spank_cred, 1);

int slurm_spank_user_init(spank_t sp, int ac, char **av) {
    FILE *file;

    file = fopen("/home/test/tmpfile", "w");
    if (file == NULL) {
        slurm_error("Failed to open /home/test/tmpfile for writing");
        return 1;
    }
    pid_t pid;
    uid_t uid;
    pid = getpid();
    uid = geteuid();

    fprintf(file, "This is test\n");
    fprintf(file,"PID: %d\n", pid);
    fprintf(file,"UID: %d\n", uid);

    fclose(file);

    return 0;
}
