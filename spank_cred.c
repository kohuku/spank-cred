#include <slurm/spank.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <errno.h>
#include <sys/time.h>
#include <time.h>



#define PORT 12345
#define BUFFER_SIZE 1024

SPANK_PLUGIN(spank_cred, 1);

int send_message_to_dpu(char *message){
    int sock = 0;
    struct sockaddr_in serv_addr;
    struct timeval timeout;
    timeout.tv_sec = 5;  // タイムアウト秒
    timeout.tv_usec = 0; // タイムアウトマイクロ秒


    // ソケットの作成
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        slurm_error("\n ソケット作成エラー \n");
        return -1;
    }

    // ソケットにタイムアウトを設定
    if (setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout)) < 0) {
        slurm_error("\n送信タイムアウト設定エラー: %s\n", strerror(errno));
        close(sock);
        return 0;
    }

    if (setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0) {
        slurm_error("\n受信タイムアウト設定エラー: %s\n", strerror(errno));
        close(sock);
        return 0;
    }

    // サーバーのアドレスとポートを設定
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    if (inet_pton(AF_INET, "192.168.100.2", &serv_addr.sin_addr) <= 0) {
        slurm_error("\n無効なアドレスまたはアドレスの種類がサポートされていません \n");
        close(sock);
        return 0;
    }

    // サーバーに接続
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        slurm_info("DPUとの接続失敗: %s", strerror(errno));
        close(sock);
        return 0;
    }
    

    // データを送信
    send(sock, message, strlen(message), 0);

    // ソケットを閉じる
    close(sock);
    return 0;
}
int slurm_spank_user_init(spank_t sp, int ac, char **av) {

    pid_t pid;
    uid_t uid;
    uint32_t job_id;
    char message[BUFFER_SIZE];  // 送信するデータ
    spank_err_t err;

    // プロセスIDとユーザーIDの取得
    pid = getpid();
    uid = geteuid();
    
    
    err = spank_get_item(sp,S_JOB_ID,&job_id);
    seteuid(0);
    snprintf(message, BUFFER_SIZE, "%d %d %d", pid, uid, job_id);
    send_message_to_dpu(message);
    seteuid(uid);

    return 0;
}

int slurm_spank_exit(spank_t sp, int ac, char **av) {
    pid_t pid;
    uid_t uid;
    char message[BUFFER_SIZE];  // 送信するデータ
    pid = getpid();
    uid = geteuid();
    struct timeval tv;
    struct tm *timeinfo;

    // 現在時刻の取得（秒とマイクロ秒）
    gettimeofday(&tv, NULL);
    timeinfo = localtime(&tv.tv_sec);  // ローカルタイムに変換
    slurm_info("PID: %d, Time: %04d-%02d-%02d %02d:%02d:%02d.%06ld",
             pid,
             timeinfo->tm_year + 1900, timeinfo->tm_mon + 1, timeinfo->tm_mday,
             timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec,
             tv.tv_usec);  // マイクロ秒をミリ秒に変換

    seteuid(0);
    snprintf(message, BUFFER_SIZE, "%d",pid);
    send_message_to_dpu(message);
    seteuid(uid);
    return 0;
}
