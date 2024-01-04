#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_BME280.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// 定义OLED显示屏的宽度和高度，单位为像素
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
// 定义OLED显示屏的I2C地址
#define OLED_I2C_ADDRESS 0x3C

// 创建BME280和OLED对象
Adafruit_BME280 bme;
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// 创建MPU6050对象
Adafruit_MPU6050 mpu;

void setup() {
    Serial.begin(9600);
}

// 步数检测阈值和步数计数变量
int stepCount = 0;
float stepThreshold = 15.0;

// 定义LED引脚
const int ledPin = A1; // 根据您的实际连接修改引脚号

void blinkLED(int times, int duration) {
  for (int i = 0; i < times; i++) {
    digitalWrite(ledPin, HIGH);   // 打开LED
    delay(duration);               // 保持一段时间
    digitalWrite(ledPin, LOW);    // 关闭LED
    if (i < times - 1) {
      delay(duration);             // 闪烁间的暂停
    }
  }
}

bool detectStep(float x, float y, float z) {
  static float lastX, lastY, lastZ;
  static int stepsDetected = 0;
  
  // 计算加速度变化量
  float deltaX = abs(x - lastX);
  float deltaY = abs(y - lastY);
  float deltaZ = abs(z - lastZ);
  
  // 更新上一次的加速度值
  lastX = x;
  lastY = y;
  lastZ = z;
  
  // 检测是否有明显的加速度变化，若有，则认为是步伐
  if (deltaX + deltaY + deltaZ > stepThreshold) {
    stepsDetected++;
    if (stepsDetected == 2) { // 两次连续的变化认为是一步
      stepsDetected = 0;
      return true;
    }
  } else {
    // 如果没有连续的变化，重置步伐检测
    stepsDetected = 0;
  }
  
  return false;
}


void setup2() {
  // 初始化串口通信
  Serial.begin(9600);

  // 初始化MPU6050
  if (!mpu.begin()) {
    Serial.println("MPU6050 not found");
    while (1);
  }
  
  // 初始化BME280
  if (!bme.begin(0x76)) {   // 根据您的实际I2C地址修改
    Serial.println("BME280 not found");
    while (1);
  }
  
  // 初始化OLED显示屏
  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_I2C_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;); // 不要继续进行，无法初始化OLED
  }
  display.display(); // 显示Adafruit logo
  delay(2000); // 延时2秒
  display.clearDisplay(); // 清除显示缓冲区

  // 初始化LED引脚为输出模式
  pinMode(ledPin, OUTPUT);
  Serial.println("Init success");
}
void loop(){
    Serial.print("Hello Moni");
    delay(1000);
}
void loop2() {
  // 读取MPU6050的加速度传感器数据
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);

  // 通过加速度变化检测步伐
  if (detectStep(a.acceleration.x, a.acceleration.y, a.acceleration.z)) {
    stepCount++;
  }

  // 读取并显示温度
  float temperature = bme.readTemperature();

  // 当步数达到10时，执行闪烁
  if (stepCount >= 5 || temperature >= 30.0) {
    blinkLED(3, 1000); // LED闪烁三次，每次500毫秒
    stepCount = 0;    // 闪烁后步数重置为0
  }



  // 更新OLED显示
  display.clearDisplay();
  display.setTextSize(2); // 设置文本大小
  display.setTextColor(SSD1306_WHITE); // 设置文本颜色
  display.setCursor(0,0); // 设置文本开始位置
  display.print("Steps: ");
  display.println(stepCount);
  display.print("Temp: ");
  display.print(temperature);
  display.println(" C");
  display.display(); // 实际显示文本

  delay(100); // 简单的去抖延时
}
