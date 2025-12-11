#include "BLEManager.h"
#include "esp_gap_ble_api.h"

// Security callbacks untuk handle pairing
class MyBLESecurityCallbacks : public BLESecurityCallbacks
{
  uint32_t onPassKeyRequest()
  {
    return 123456; // PIN untuk pairing (6 digit)
  }

  void onPassKeyNotify(uint32_t pass_key)
  {
  }

  bool onConfirmPIN(uint32_t pass_key)
  {
    return true;
  }

  bool onSecurityRequest()
  {
    return true;
  }

  void onAuthenticationComplete(esp_ble_auth_cmpl_t cmpl)
  {
  }
};

MyBLEServerCallbacks::MyBLEServerCallbacks(BLEManager *mgr) : manager(mgr)
{
}

void MyBLEServerCallbacks::onConnect(BLEServer *pServer)
{
  delay(10);
  manager->setDeviceConnected(true);
  manager->lastEventTime = millis();
  
  // Call callback jika sudah di-set
  if (manager->onConnectCallback)
  {
    manager->onConnectCallback();
  }
}

void MyBLEServerCallbacks::onDisconnect(BLEServer *pServer)
{
  delay(10);
  manager->setDeviceConnected(false);
  
  // Call callback jika sudah di-set
  if (manager->onDisconnectCallback)
  {
    manager->onDisconnectCallback();
  }
}

MyBLECharacteristicCallbacks::MyBLECharacteristicCallbacks(BLEManager *mgr) : manager(mgr)
{
}

void MyBLECharacteristicCallbacks::onRead(BLECharacteristic *pCharacteristic)
{
  // Read request - tidak perlu handle khusus
}

void MyBLECharacteristicCallbacks::onWrite(BLECharacteristic *pCharacteristic)
{
  std::string value = pCharacteristic->getValue();
  if (value.length() > 0)
  {
    String eventData = String(value.c_str());
    manager->handleEvent(eventData);
  }
}

BLEManager::BLEManager()
{
  pServer = nullptr;
  pService = nullptr;
  pCharacteristic = nullptr;
  deviceConnected = false;
  oldDeviceConnected = false;
  pCallbacks = nullptr;
  pCharCallbacks = nullptr;
  lastEventTime = 0;
  onMessageCallback = nullptr;
  onConnectCallback = nullptr;
  onDisconnectCallback = nullptr;
}

void BLEManager::begin(const char *deviceName)
{
  BLEDevice::init(deviceName);

  // Set security callbacks untuk handle pairing
  BLEDevice::setEncryptionLevel(ESP_BLE_SEC_ENCRYPT);
  BLEDevice::setSecurityCallbacks(new MyBLESecurityCallbacks());

  // Konfigurasi security - NoInputNoOutput (tidak perlu PIN manual)
  BLESecurity *pSecurity = new BLESecurity();
  pSecurity->setAuthenticationMode(ESP_LE_AUTH_BOND);
  pSecurity->setCapability(ESP_IO_CAP_NONE);
  pSecurity->setInitEncryptionKey(ESP_BLE_ENC_KEY_MASK | ESP_BLE_ID_KEY_MASK);

  // Set MTU
  BLEDevice::setMTU(247);

  pServer = BLEDevice::createServer();

  pCallbacks = new MyBLEServerCallbacks(this);
  pServer->setCallbacks(pCallbacks);

  // Gunakan UUID custom (128-bit) untuk menghindari konflik dengan UUID standar
  BLEUUID serviceUUID("1b5dccd3-ef4d-4df0-94b2-7f411f1e0844");
  pService = pServer->createService(serviceUUID);

  // UUID custom untuk characteristic
  BLEUUID charUUID("1999eae0-d5ad-4909-aff3-4a8875149db5");
  pCharacteristic = pService->createCharacteristic(
      charUUID,
      BLECharacteristic::PROPERTY_READ |
          BLECharacteristic::PROPERTY_WRITE |
          BLECharacteristic::PROPERTY_WRITE_NR |
          BLECharacteristic::PROPERTY_NOTIFY);

  // Set callbacks untuk handle read/write requests
  pCharCallbacks = new MyBLECharacteristicCallbacks(this);
  pCharacteristic->setCallbacks(pCharCallbacks);

  // Buat descriptor dengan callback untuk handle subscription
  BLE2902 *pDescriptor = new BLE2902();
  pCharacteristic->addDescriptor(pDescriptor);

  // Set initial value
  pCharacteristic->setValue("PetRobot Ready");

  pService->start();

  // Set device name di GAP (Generic Access Profile)
  BLEDevice::setEncryptionLevel(ESP_BLE_SEC_ENCRYPT);
  
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(serviceUUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06);
  pAdvertising->setMinPreferred(0x12);
  // Connection interval yang lebih stabil
  pAdvertising->setMinInterval(0x20); // 32 * 1.25ms = 40ms
  pAdvertising->setMaxInterval(0x40); // 64 * 1.25ms = 80ms
  BLEDevice::startAdvertising();
}

void BLEManager::update()
{
  // Handle disconnection
  if (!deviceConnected && oldDeviceConnected)
  {
    delay(500); // Give the bluetooth stack the chance to get things ready
    pServer->startAdvertising(); // Restart advertising
    oldDeviceConnected = deviceConnected;
  }

  // Handle new connection
  if (deviceConnected && !oldDeviceConnected)
  {
    lastEventTime = millis();
    oldDeviceConnected = deviceConnected;
  }
}

bool BLEManager::isConnected()
{
  return deviceConnected;
}

void BLEManager::sendData(String data)
{
  if (deviceConnected && pCharacteristic != nullptr && pServer != nullptr)
  {
    // Check connection status
    if (pServer->getConnectedCount() == 0)
    {
      deviceConnected = false;
      return;
    }
    
    pCharacteristic->setValue(data.c_str());
    pCharacteristic->notify();
  }
}

void BLEManager::setDeviceConnected(bool connected)
{
  deviceConnected = connected;
}

void BLEManager::handleEvent(String event)
{
  lastEventTime = millis();
  
  // Call callback jika sudah di-set
  if (onMessageCallback)
  {
    onMessageCallback(event);
  }
  
  // Auto response untuk ping (optional)
  if (event == "ping" || event.startsWith("ping:"))
  {
    sendData("pong");
  }
}

void BLEManager::setOnMessageCallback(std::function<void(String)> callback)
{
  onMessageCallback = callback;
}

void BLEManager::setOnConnectCallback(std::function<void()> callback)
{
  onConnectCallback = callback;
}

void BLEManager::setOnDisconnectCallback(std::function<void()> callback)
{
  onDisconnectCallback = callback;
}

