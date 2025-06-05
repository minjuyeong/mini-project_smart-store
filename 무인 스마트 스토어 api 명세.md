## 무인 스마트 스토어 api 명세

1. 매장(STM32) -> 서버
    1. 자동문 제어
        - [A_DOOR]DOOR@OPEN
2. 서버 -> 매장(STM32)
    1. 경비실로부터 오는 명령
        1. 모든 장치 중단
            - [A]ALLSTOP@ON
                - 모든 장치 중단
            - [A]ALLSTOP@OFF
                - 모든 장치 중단 취소
        2. 현재 매장 상태
            - [A]STATE
        3. FAN 속도 조절
            - [A]FAN@팬속도
                - 팬 속도 : 0 ~ 1000
        4. 온습도 센서 데이터
            - [A]DHTSTATE@온도@습도
        5. 현재 조명 상태 및 조절
            - [A]LED@STATE
            - [A]LED@밝기
                - 밝기 : 0 ~ 100