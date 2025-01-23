# OBD2_Interface_v2_Release
 Released Version

$${\color{red}Disclaimer:}$$  
THIS SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED  
TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, AND NONINFRINGEMENT. IN NO EVENT SHALL  
THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES, OR OTHER LIABILITY, WHETHER IN AN ACTION OF  
CONTRACT, TORT, OR OTHERWISE, ARISING FROM, OUT OF, OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER  
DEALINGS IN THE SOFTWARE.  

THIS SOFTWARE AND HARDWARE ARE PROVIDED FOR EDUCATIONAL AND EXPERIMENTAL PURPOSES ONLY. IT IS THE USER'S  
RESPONSIBILITY TO ENSURE COMPLIANCE WITH ALL APPLICABLE LAWS AND REGULATIONS REGARDING VEHICLE DATA ACCESS AND  
MODIFICATION.  

USE THIS DEVICE AND SOFTWARE AT YOUR OWN RISK. INTERACTING WITH VEHICLE SYSTEMS MAY RESULT IN UNINTENDED  
CONSEQUENCES, INCLUDING BUT NOT LIMITED TO ERRORS, SYSTEM MALFUNCTIONS, OR DATA LOSS. THE AUTHORS ACCEPT NO  
RESPONSIBILITY FOR DAMAGE TO VEHICLES, DATA, OR OTHER PROPERTY RESULTING FROM THE USE OF THIS DEVICE OR SOFTWARE.  


$${\color{red}Important \space Note:}$$  
**NEVER Connect** to a vehicle before you have checked the settings to match the CAN Bus speeds of the car:
![image](https://github.com/user-attachments/assets/934fe07c-86f1-4d9f-8fb0-78302130e049)

And decided if you want to allow the device to send commands, AKN and ERR messages to the car:
![image](https://github.com/user-attachments/assets/d4d66572-2c5e-4612-9199-1a4dbfd85141)


${\color{blue}The \space OBD2_Interface_v2 \space (unboxed \space so \space you \space can \space see \space the \space requirements \space to \space build \space yourself):}$  

![OBD2_Interface_v2_Circuit_Photo](https://github.com/user-attachments/assets/3884fbb4-028c-455c-a5a6-b9f745f8d333)


${\color{blue}The \space OBD2_Interface_v2 \space Functions:}$  

![image](https://github.com/user-attachments/assets/8a8de787-cd0b-43aa-b506-beddd2631a25)

Save CAN Bus Data from both High Speed and Medium Speed buses onto SD Card and load into SavvyCAN for analysis  
![image](https://github.com/user-attachments/assets/d5268628-d219-415c-b9ad-b7994a49e89f)

![image](https://github.com/user-attachments/assets/78b7f6fd-10b4-4932-aff5-8ba98f14cf12)


$${\color{blue}Connections \space and \space Equipment:}$$  
// Connections to the ESP32-S3 Follow:  
  Power 5v Switched Supply  
  GND = GND  
  12v = VCC In  
  5v Out = 5v  
  
  ESP32-S3 & two MCP2515 Breakout Board (HW-184) - 8 MHz Crystal  
  MCP2515_0 for 500kbps CAN Bus  
  SCK = pin 12  
  SI = pin 11  
  SO = pin 13  
  CS = pin 14  
  GND = GND  
  VCC = 5v  

  MCP2515_1 for 125kbps CAN Bus  
  SCK = pin 12  
  SI = pin 11  
  SO = pin 13  
  CS = pin 10  
  GND = GND  
  VCC = 5v  

  ESP32-S3 OpenLager (SD Card) Connections ("OpenLager" device should not be confused with the slower "OpenLogger")   
  Rx = Pin 9  
  GND = GND  
  VCC = 5v  

  ESP32-S3 ILI9341 Display (with touch) Connections  
  T_IRQ = Not Connected  
  T_DO = 38  
  T_DIN = 41  
  T_CS = 37  
  T_CLK = 40  
  MISO = 38  
  LED = 39 (2.5mA)  
  SCK = 40  
  MOSI = 41  
  DC = 42  
  RESET = 2  
  CS = 1  
  GND = GND  
  VCC = 3.3v  

  Stop Button  
  GND = GND  
  Botton = 21  


  
