#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <errno.h>

#define PORT 12345
#define BUFFER_SIZE 1024


int main() {

    pid_t pid;
    uid_t uid;
    pid = getpid();
    uid = geteuid();


    int sock = 0;
    struct sockaddr_in serv_addr;
    char message[BUFFER_SIZE];  // 送信するデータ
    char buffer[BUFFER_SIZE] = {0};
    struct timeval timeout;
    timeout.tv_sec = 5;  // タイムアウト秒
    timeout.tv_usec = 0; // タイムアウトマイクロ秒

    snprintf(message, BUFFER_SIZE, "%d %d", pid, uid);

    // ソケットの作成
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("\n ソケット作成エラー \n");
        return -1;
    }

    // ソケットにタイムアウトを設定
    if (setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout)) < 0) {
        printf("\n送信タイムアウト設定エラー: %s\n", strerror(errno));
        close(sock);
        return 0;
    }

    if (setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0) {
        printf("\n受信タイムアウト設定エラー: %s\n", strerror(errno));
        close(sock);
        return 0;
    }

    // サーバーのアドレスとポートを設定
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    if (inet_pton(AF_INET, "192.168.100.2", &serv_addr.sin_addr) <= 0) {
        printf("\n無効なアドレスまたはアドレスの種類がサポートされていません \n");
        close(sock);
        return 0;
    }

    // サーバーに接続
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("\n接続失敗: %s\n", strerror(errno));
        close(sock);
        return 0;
    }

    FILE *file;

    file = fopen("/home/test/tmpfile", "w");
    if (file == NULL) {
        printf("Failed to open /home/test/tmpfile for writing");
        return 1;
    }
    // データを送信
    send(sock, message, strlen(message), 0);
    fprintf(file,"メッセージが送信されました: %s\n", message);
    // ソケットを閉じる
    close(sock);


    return 0;
}
