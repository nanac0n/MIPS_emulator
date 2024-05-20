cmake_minimum_required(VERSION 3.10)  # CMake 최소 버전 지정
project(PipeSim)  # 프로젝트 이름 설정

# C 표준 버전 설정
set(CMAKE_C_STANDARD 11)

# 헤더 파일이 있는 디렉토리를 포함
include_directories(${PROJECT_SOURCE_DIR})

# 실행 파일 생성을 위한 소스 파일 지정
add_executable(PipeSim main.c pa3.c)

# 여기서 추가 설정을 할 수 있습니다.
# 예를 들어, 특정 컴파일러 옵션을 추가하거나, 링크할 라이브러리가 있다면 설정할 수 있습니다.
# 예시: 'testcases' 폴더 내의 모든 파일을 빌드 디렉토리의 'testcases' 폴더로 복사
file(GLOB TESTCASES_FILES "${CMAKE_SOURCE_DIR}/testcases/*")
foreach(FILE ${TESTCASES_FILES})
    file(COPY ${FILE} DESTINATION "${CMAKE_BINARY_DIR}/testcases")
endforeach()