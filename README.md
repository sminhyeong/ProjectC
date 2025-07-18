주제 : 멀티 보스 레이드 컨텐츠(메이플, 로스트 아크, 마비노기 모바일 등에 있는 레이드 컨텐츠 구현)
프로젝트 기간 : 2025년 7월 1일 ~ 2025년 7월 18일

사용한 툴 : UnrealEngine 5.5.4 Ver

게임 구조 : 리슨서버형식의 게임 서버 및 클라이언트
===============================================================================================
*게임 기능*
1. Tiltle Map : 로그인, 회원 가입, 게임 서버 생성, 게임서버 게임 서버 리스트 확인, 게임 서버 진입
2. Lobby Map : 상점, 아이템 구매 및 판매, 던전 진입 포탈
3. Dungeon Map : Boss Monster, 입구 옆 로비로 탈출용 포탈, Boss전 클리어 시 개인별 보상 상장, Boss전 클리어 후 던전 나가기 포탈
4. 플레이어 기능 : 스킬창, 아이템 창, 스킬 선택, 스킬 사용, 아이템 정보 보기, 아이템 사용, 상점 NPC및 보상 상자와 상호작용기능
5. 스킬 종류 : 원거리 투사체 스킬(ex 파이어볼), 회복 스킬(광역힐), 스킬을 맞은 대상에게 돌진하는 스킬(ex LOL 사일러스 E스킬), 스킬을 맞은 대상을 끌어오는 스킬(ex LOL 블리츠 크랭크 Q스킬)
6. 플레이어 및 몬스터 Death 처리 : Hp가 0 이하로 감소할 경우 ragdoll을 True로 변경하고 움직이지 못하게 처리.
7. 몬스터가 플레이어 인식 시 Boss Hp Bar 표시 및 Boss 사망 시 Boss Hp Bar 비표시로 변경

DB서버 연동 처리(DB Account Server Git : https://github.com/sminhyeong/GameDatabaseServer.git)
Item관련 데이터 처리

C++ DB Account서버와 언리얼 엔진간의 TCP통신을 통한 데이터 주고 받는 형식
통신은 언리얼엔진의 Game Mode와 DB서버간 통신을 통해 데이터 주고 받도록 구성

사용한 Packet: FlatBuffer를 이용하여 Packet을 구성하여 DB서버와 통신
(Flat Buffer git: https://github.com/google/flatbuffers/releases)

사용한 에셋

공격 애니메이션
https://www.fab.com/listings/b81b2df8-d7fd-4d16-a303-0b73c7053cdb

무기
https://www.fab.com/listings/d5be0dc9-1a41-4be2-a63a-5ed436f3445d

로비 및 전투맵 하늘
Cartoon Sky Shader - https://www.fab.com/listings/2bcc9fb8-fb7e-45ad-95b5-caa95679c408

로비 맵
Stylized Catcafe 110 Asset Pack - https://www.fab.com/listings/bd81ea3d-9a9e-4c0f-a847-cadb5cc46276

인벤토리 및 스토어
GUI Parts - https://www.fab.com/ko/listings/695e149d-93e3-42bb-b021-4f1bbf6eac2a

보스 몬스터
Paragon: Sevarog - https://www.fab.com/listings/a4882b5e-cfad-4830-a3dd-46a6c31a79b2

플레이어
Creative Characters FREE - Animated Low Poly 3D Models - https://www.fab.com/listings/94fd60a2-5659-4fc4-af1d-a8cdd2681c2e

스킬 아이콘
https://www.leagueoflegends.com/ko-kr/champions/sylas/ 사일러스 E(도주/억압)

https://www.leagueoflegends.com/ko-kr/champions/blitzcrank/ 블리츠크랭크 Q(로켓 손)

