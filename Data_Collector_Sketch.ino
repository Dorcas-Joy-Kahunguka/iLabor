#include <Ethernet.h>
#include <MySQL_Connection.h>
#include <MySQL_Cursor.h>


//const int SENSOR_PIN = A0;
//const int SAMPLING_RATE = 200; // Sampling rate in milliseconds (5Hz)

// Forward declarations to avoid circular dependencies
class DataPoint {
public:
  static const int NUM_SAMPLES = 5;

  DataPoint() : timestamp(0) {}

  void addSample(int sample) {
    if (sampleCount < NUM_SAMPLES) {
      samples[sampleCount++] = sample;
    }
  }

  bool isComplete() const {
    return sampleCount == NUM_SAMPLES;
  }

  void reset() {
    sampleCount = 0;
  }

  float getSample(int index) const {
    return (index < NUM_SAMPLES) ? samples[index] : 0.0;
  }

  void setTimestamp(unsigned long time) {
    timestamp = time;
  }

  unsigned long getTimestamp() const {
    return timestamp;
  }

private:
  int samples[NUM_SAMPLES];
  int sampleCount = 0;
  unsigned long timestamp;
};

class DataStorage {

public:
  DataStorage(Client &client) : conn(new MySQL_Connection(&client)) {
    // Initialize your MySQL connection parameters
    IPAddress my_server_ip(127, 0, 0, 1); // MySQL server IP address
    char user[] = "root";
    char password[] = "iLabor_MLxSensor23";
    char db[] = "ilabor";

    // Connect to MySQL
    if (conn->connect(my_server_ip, 3306, user, password, db)) {
      Serial.println("Connected to MySQL server");
    } 
    else {
      Serial.println("Connection to MySQL server failed");
      
    }
    
  }
  
  void storeDataPoint(DataPoint &dataPoint) {
    char query[255];
    unsigned long dataPointID;
    // Insert into data_point_table to get the data point ID
    sprintf(query, "INSERT INTO raw_data_point (timestamp) VALUES (%lu)", dataPoint.getTimestamp());

    MySQL_Cursor cur_mem(conn);
    cur_mem.execute(query);

    // Retrieve the generated data point ID
    sprintf(query, "SELECT LAST_INSERT_ID() AS last_inserted_id");
    cur_mem.execute(query);

    dataPointID = cur_mem.get_next_row();

    // Insert individual samples into sample_table
    for (int i = 0; i < DataPoint::NUM_SAMPLES; ++i) {
        sprintf(query, "INSERT INTO raw_data_sample (raw_dpt_id, value) VALUES (%lu, %d)", dataPointID, dataPoint.getSample(i));

         //cur_mem = new MySQL_Cursor(&conn);
        cur_mem.execute(query);
         //cur_mem.execute(query);
    }
    
  }
    

private:
  MySQL_Connection *conn;
};


//class DataStorage;
//class DataPoint;

class DataCollector {
public:
  DataCollector(DataStorage &storage,  DataPoint &dataPoint) : storage(storage), dataPoint(dataPoint) {}

  void setup() {
  // No specific setup for sensor pin is required if using A0 which is the default analog input
  // otherwise set  pinMode(SENSOR_PIN, INPUT); 
  // Initialize serial communication for debugging or monitoring
  Serial.begin(9600);
  Serial.println("Beginning processing");
  }

  void loop() {
    int sensorReading = readSensor();
    Serial.println("Inside loop");
    // Store readings to form a data point
     dataPoint.addSample(sensorReading);
    Serial.println("Added data");
    // If a data point is complete, process it and reset index
    if (dataPoint.isComplete()) {
       Serial.println("Attempting to store"); 
      processDataPoint();
  Serial.println("Waiting to begin");
      // Delay to achieve the desired sampling rate
      delay(200);

      // Reset dataPoint object for the next iteration
     dataPoint.reset();
    
     }
  }

  float readSensor() {
    // Your sensor reading code
    // Example: return analogRead(SENSOR_PIN);
    return random(50); // Placeholder for testing
  }

  void processDataPoint() {
    // Your processing code

    // Send data point for storage
  storage.storeDataPoint(dataPoint);
  }

private:
  // Reference to the DataStorage instance
  DataStorage &storage;
  DataPoint &dataPoint;
};


// Create instances with dependency injection
DataPoint dataPoint;
EthernetClient client;
DataStorage storage(client);
DataCollector dataCollector(storage, dataPoint);

void setup() {
  dataCollector.setup();
}

void loop() {
  dataCollector.loop();
}
