# socketTest

1:n socket 통신 필요.

싱글스레드 -> 멀티스레드 (문제발생) -> i/o멀티플렉싱 도입 (select) but 최대 64개의 handler밖에 처리 x

# client-server


# i.o 멀티플렉싱
- 모든 입/출력은 프로세스가 아니라 os가 주도한다는 것이 핵심

- 여러 입/출력 요청이 한 채널에 동시에 혼재 할 수 있는 입/출력 구조

