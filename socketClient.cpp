#include <cstring>
#include <iostream>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdio.h>
#include <arpa/inet.h>

using namespace std;
const int port = 9032;
const int maxConn = SOMAXCONN;
const char* serverADDR = "127.0.0.1";
#define BUF_SIZE 128;

int main(void){

    //ipv4와 tcp를 이용한 소켓client 생성
    int clientSocket = socket(AF_INET, SOCK_STREAM,0);

    sockaddr_in serverAddress = { 0 };
    serverAddress.sin_family = AF_INET; //ipv4
    serverAddress.sin_port = htons(port); //9032 포트
    serverAddress.sin_addr.s_addr = inet_addr(serverADDR); //연결받을 주소

    //소켓 연결 시작.
    //connection이 중간에 서버 다운으로 끊어졌을때, conn값이 계속 유지되어 client가 꺼지지 않고
    //server가 재기동 되지 않는 문제가 존재.
    int isConn = connect(clientSocket,(struct sockaddr*)&serverAddress, sizeof(serverAddress));
    if(isConn ==-1){
        //에러코드 반환 (추후 에러관련 내용을 에러 코드를 반환하고, 로그에 저장하도록 개선.)
        puts("ERROR : 서버와 연결할 수 없습니다."); fflush(stdout);
        return 0;
    }
    //데이터 입력
    char buf[128]={0};
    sockaddr addrInfo;
    socklen_t addrLen;

    while(1){
        //입력
        fgets(buf,sizeof(buf), stdin); fflush(stdin);
        buf[strcspn(buf, "\n")] = '\0';
        if(strcmp(buf,"EXIT")==0){
            memset(buf,0,sizeof(buf));
            break;
        }
        //데이터 전달. -> buff 부분에서 protocol 정의하여, 전송할 내용 전달할 것.
        send(clientSocket,buf,strlen(buf),0);
        //버퍼 초기화.
        memset(buf,0,sizeof(buf));
        
        recv(clientSocket,buf,sizeof(buf),0);
        cout<<"message from server : ";
        puts(buf);
        memset(buf,0,sizeof(buf));
    }
    //client에서 shutdown 전송.
    //shutdown(clientSocket,SHUT_RDWR);
    //전송 종료.
    close(clientSocket);

    return 0;
}