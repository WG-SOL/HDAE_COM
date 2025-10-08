# 컴파일러 설정
CXX = g++
CXXFLAGS = -std=c++11 -Wall

# 링커 라이브러리 설정
LIBS = -lmosquittopp

# 소스 파일 목록
SRCS = main.cpp mqtt_gateway.cpp can_tp_sender.cpp

# 오브젝트 파일 목록 (소스 파일 확장자를 .o로 변경)
OBJS = $(SRCS:.cpp=.o)

# 실행 파일 이름
TARGET = mqtt_to_can

# 기본 빌드 규칙
all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS) $(LIBS)

# .cpp 파일을 .o 파일로 컴파일하는 규칙
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# 중간 파일 정리
clean:
	rm -f $(OBJS) $(TARGET)
