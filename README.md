주제 : 멀티 보스 레이드 컨텐츠(메이플, 로스트 아크, 마비노기 모바일 등에 있는 레이드 컨텐츠 구현)

사용한 툴 : UnrealEngine 5.5.4 Ver

게임 구조 : 리슨서버형식의 게임 서버 및 클라이언트
===============================================================================================
*게임 기능*
1. Tiltle Map : 로그인, 회원 가입, 게임 서버 생성, 게임서버 게임 서버 리스트 확인, 게임 서버 진입
2. Lobby Map : 상점, 아이템 구매 및 판매, 던전 진입 포탈
3. Dungeon Map : Boss Monster, 입구 옆 로비로 탈출용 포탈, Boss전 클리어 시 개인별 보상 상장, Boss전 클리어 후 던전 나가기 포탈
4. 플레이어 기능 : 스킬창, 아이템 창, 스킬 선택, 스킬 사용, 아이템 정보 보기, 아이템 사용, 상점 NPC및 보상 상자와 상호작용기능
5. 스킬 종류 : 원거리 투사체 스킬(ex 파이어볼), 회복 스킬(광역힐), 스킬을 맞은 대상에게 돌진하는 스킬(ex LOL 사일러스 E스킬), 스킬을 맞은 대상을 끌어오는 스킬(ex LOL 블리츠 크랭크 Q스킬)
6. 플레이거 및 몬스터 Death 처리 : Hp가 0 이하로 감소할 경우 ragdoll을 True로 변경하고 움직이지 못하게 처리.

DB서버 연동 처리(DB Account Server Git : https://github.com/sminhyeong/GameDatabaseServer.git)
Item관련 데이터 처리

C++ DB Account서버와 언리얼 엔진간의 TCP통신을 통한 데이터 주고 받는 형식
통신은 언리얼엔진의 Game Mode와 DB서버간 통신을 통해 데이터 주고 받도록 구성

사용한 Packet: FlatBuffer를 이용하여 Packet을 구성하여 DB서버와 통신
(Flat Buffer git: https://github.com/google/flatbuffers/releases)
