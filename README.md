# 현대오토에버 모빌리티 SW스쿨 2기_Embedded

## TEAM 1

## Team
<table>
  <tr>
    <td align="center">1</td>
    <td align="center">2</td>
    <td align="center">3</td>
    <td align="center">4</td>
    <td align="center">5</td>
  </tr>
     <tr>
    <td align="center"><a href="https://github.com/"><sub><b>기동언</b></td>
    <td align="center"><a href="https://github.com/"><sub><b>유원규</b></td>
    <td align="center"><a href="https://github.com/"><sub><b>심동현</b></td>
    <td align="center"><a href="https://github.com/"><sub><b>이나연</b></td>
    <td align="center"><a href="https://github.com/"><sub><b>김종훈</b></td>  
  </tr>
      
</table>

<br><br>

## 🛠 Stack

### 💻 Languages
![C](https://img.shields.io/badge/C-%2300599C.svg?style=for-the-badge&logo=c&logoColor=white)
![C++](https://img.shields.io/badge/C++-%2300599C.svg?style=for-the-badge&logo=c%2B%2B&logoColor=white)

### 🔧 Embedded System
![Embedded](https://img.shields.io/badge/Embedded-%231572B6.svg?style=for-the-badge&logo=platformdotio&logoColor=white)
![Infineon](https://img.shields.io/badge/Infineon-A8B400.svg?style=for-the-badge&logo=infineon&logoColor=white)

### ⚙️ Tools
![Git](https://img.shields.io/badge/Git-F05032.svg?&style=for-the-badge&logo=Git&logoColor=white)
![AURIX Studio](https://img.shields.io/badge/AURIX%20Studio-0088CC.svg?style=for-the-badge)
![UDE Platform](https://img.shields.io/badge/UDEPlatform-D2232A.svg?style=for-the-badge)


## ✔️ 주요 기능

1. Mobile App과 RPI(HPC) 사이 BLE를 연결 후 원격조작 및 램프 스위치 기능

2. SDV 전환에 맞춰 중앙 집중화. RPI가 SOME/IP를 통해 각각의 Domain ECU에 Request/Response를 받아 HW Service 제어 서비스를 RPC로 수행

3. DoIP, DTC 등 진단. PC에서 GUI로 진단기를 통해 진단

4. OTA 기능

5. RTOS를 통한 실시간 제어 및 태스크, 자원 관리

## Architecture

### SYSTEM Architecture
<img width="677" height="457" alt="Image" src="https://github.com/user-attachments/assets/5abaef62-585d-4773-bde2-c3e5b1fd21a9" />

### SOME/IP Architecture

<img width="545" height="280" alt="Image" src="https://github.com/user-attachments/assets/1ce096bb-3460-4f21-b528-a60b11ac94bc" />
<img width="798" height="657" alt="Image" src="https://github.com/user-attachments/assets/80d517da-3efa-4957-92e0-50fa2da8b0c7" />

### OTA Architecutre

<img width="515" height="328" alt="Image" src="https://github.com/user-attachments/assets/72d3a206-6fc9-4861-9666-9fd56efcc0ea" />

<img width="1000" height="308" alt="Image" src="https://github.com/user-attachments/assets/f0b65fc5-ff2c-42e5-a7d9-2905fa32e36f" />

### Diagnostics Architecture

<img width="503" height="342" alt="Image" src="https://github.com/user-attachments/assets/16f7a0e6-69bf-445b-a66e-78e43eb00918" />

### Body Diagnostics

<img width="812" height="397" alt="Image" src="https://github.com/user-attachments/assets/cdea38f6-3eb1-4d0f-828d-688dfcd42c47" />

### Chassis Diagnostics

<img width="820" height="456" alt="Image" src="https://github.com/user-attachments/assets/1ec27148-0d7a-47ca-836b-0963f5a41fdf" />

### Diagnostics



### CAN DBC

<img width="727" height="355" alt="Image" src="https://github.com/user-attachments/assets/11701b46-b109-470c-a38f-411366b55cac" />


### RTOS

<img width="868" height="417" alt="Image" src="https://github.com/user-attachments/assets/92edc8db-2d05-4d28-af7e-7c9cf60d410e" />

<img width="511" height="417" alt="Image" src="https://github.com/user-attachments/assets/1354579d-c79c-4b68-88bb-4a6f844ce76f" />

<img width="891" height="432" alt="Image" src="https://github.com/user-attachments/assets/28a1e889-8005-4c81-be38-6aa48c711466" />


<br>



## 🤙🏻 commit 컨벤션

```
💡 feat : 새로운 기능 추가
🐞 fix : 버그 수정
📄 docs : 문서 수정
🛠 refact : 코드 리팩토링
💅 style : 코드 의미에 영향을 주지 않는 변경사항
📦 chore : 빌드 부분 혹은 패키지 매니저 수정사항
```
