#include <cstring>
#include <iostream>
#include <netinet/in.h> //인터넷 프로토콜 (ipv4관련 구조체와 상수 정의 헤더)
#include <sys/socket.h> //소켓프로그래밍을 위한 기본적인 함수와 구조체 상수 정의
#include <sys/select.h> //select를 통한 멀티플렉싱 사용
#include <unistd.h> //sleep위함
#include <pthread.h>
#include <arpa/inet.h>//ntohs 사용을 위한 include
#include <vector>

using namespace std;

const int port = 9032;
#define BUF_SIZE 128;
vector<threadInfo> cthreadInfo; // thread 관리를 위한 vector
//스레드에 전달될 매개변수.
struct threadInfo{
    int clientSocket; //클라이언트 소켓 번호
    sockaddr_in clientAddr; //클라이언트 ip정보
};

//공유원에 접근하는 것을 관리하기 위해 critical section 정의가 필요.
//추후 필요에 따라 작성
bool AddClient(int clientSocket){

}

//ctrl 명령이 들어왔을때 모든 클라이언트의 연결을 종료하는 함수
//추후 필요에 따라 작성
bool CtrlHandler(){

}
//accept시 생성될 스레드.
void *acceptThread(void *arg){

    //매개변수로 부터 들어온 소켓정보활용을 위한 변수
    threadInfo *thread = (threadInfo*)arg;
    int clientSocket = thread->clientSocket;
    sockaddr_in clientAddr = thread->clientAddr;

    //client -> server로 보내는 내용에 대한 receive buffer
    //현재는 recv버퍼가 128로 128byte를 초과하는 요청은 잘려짐.
    char sRecvBuffer[128] = { 0 };

    int Recv;
    while((Recv=recv(clientSocket,sRecvBuffer,sizeof(sRecvBuffer)-1,0))>0){
        //recv의 결과로 반환받은 마지막 부분에 \0을 할당하여 종료시점을 명확하게 명시.
        sRecvBuffer[Recv] = '\0'; 

        //전달받은 내용을 서버에 표시.
        cout<<"message from client"<<clientSocket<<"\n";

        //전달받은 내용을 데이터를 전송한 client에 전송.
        send(clientSocket,sRecvBuffer,Recv,0);

        //! 추후 데이터 저장 (db) -> 임계구역 처리 필요

        //멀티플렉싱 처리 필요. (모든 client에게 메세지 전송)
        //서버에 데이터를 전송한다면 굳이 필요한 작업일까
        //채팅, 게임과 같은 응답속도가 중요한 영역에서는 필요하나
        //목적이 재고관리 통신을 위한 바, 필요없을 것으로 생각.



        //메세지 표출
        puts(sRecvBuffer);
        //메모리 초기화.
        memset(sRecvBuffer,0,sizeof(sRecvBuffer));
    }
    if(Recv == 0){
        cout << "클라이언트가 연결을 종료하였습니다.\n";
        //client에서 shutdown 읽기버퍼 쓰기버퍼 모두 종료.
        shutdown(clientSocket,SHUT_RDWR);
        
        //서버와 연결된 클라이언트의 소켓 close;
        close(clientSocket);
        
        // 메모리 해제 필수
        delete thread; 
        //쓰레드 종료.
        pthread_exit(NULL);
        puts("connection closed."); fflush(stdout);
    }else{
        perror("메세지 수신 오류");
        delete thread; // 메모리 해제 필수
        pthread_exit(NULL);
    }

}

int main(void){

    //1. 소켓 생성
    int serverSocket = socket(AF_INET,SOCK_STREAM,0);

    //2. socket 바인딩.
    sockaddr_in serverAddress = { 0 };
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(port);

    //ip 주소가 무엇인지 신경쓰지 않는다 (*ip주소는 nic 카드 갯수에 따라서 달라지므로)
    serverAddress.sin_addr.s_addr = INADDR_ANY;

    if(::bind(
        serverSocket,(sockaddr*)&serverAddress, sizeof(serverAddress)) == -1)
    {
        perror("Binding Error : 연결종료");
        return 0;
    };

    //3. 접속대기로 전환
    if(::listen(serverSocket,SOMAXCONN)==-1){
        puts("ERROR : 접속대기 상태로 전환 불가");
        return 0;
    }

    //4. 클라이언트 접속처리 및 내용
    int nRecv = 0;
    int clientSocket =0;
    sockaddr_in clientAddr = { 0 };
    socklen_t clientAddrLen = sizeof(clientAddr);


    //4. 클라이언트의 연결을 받아 새로운 소켓 생성
    //accept의 경우 socket을 위한 파일지정번호를 반환.
    while((clientSocket = accept(serverSocket, 
        (sockaddr*)&clientAddr, &clientAddrLen))!=-1)
    {
        threadInfo *tInfo = new threadInfo;
        tInfo->clientSocket = clientSocket;
        tInfo->clientAddr = clientAddr;
        pthread_attr_t cpthreadAttr;

        //새 client가 들어올 때 마다 새로운 스레드 생성
        //새 스레드 속성 오브젝트 작성.
        pthread_t threads;
        int *threadRet;
        pthread_attr_init(&cpthreadAttr);
        if(pthread_create( 
            &threads,
            NULL,
            acceptThread,
            (void *) tInfo //acceptThread로 들어갈 인자.
        )!=0){
            perror ("thread 생성 실패");
            close(clientSocket);
            continue;
        }
        else{
            cthreadInfo.push_back(tInfo);
            cout << "클라이언트와 연결이 완료되었습니다" << endl;
            cout << "Client IP: " << inet_ntoa(clientAddr.sin_addr) << endl;
            cout << "Port: " << ntohs(clientAddr.sin_port) << endl;
        };
    }
    //리슨 소켓 닫기
    close(serverSocket);

    return 0;
}