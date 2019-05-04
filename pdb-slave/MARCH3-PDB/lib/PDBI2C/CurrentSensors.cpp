#include "CurrentSensors.h"

CurrentSensors::CurrentSensors(PinName SDA_PIN, PinName SCL_PIN) : bus(SDA_PIN, SCL_PIN){
    this->ADS1015_read = (ADS1015_address << 1) | 0x01; // Shift left and set LSB to one
    this->ADS1015_write = (ADS1015_address << 1) & 0xFE; // Shift left and set LSB to zero
    this->FSR = fsr2048; // Set default Full Scale Range
    this->dataRate = dr1600sps; // Set default data rate
}

float CurrentSensors::getLSBSize(){
    switch(this->FSR){
        case fsr6144:
            return 3.0f;
        case fsr4096:
            return 2.0f;
        case fsr2048:
            return 1.0f;
        case fsr1024:
            return 0.5f;
        case fsr0512:
            return 0.25f;
        case fsr0256:
            return 0.125f;
        default:
            return 0.0f;
    }
}

uint16_t CurrentSensors::swapBytes(uint16_t data){
    uint8_t MSB, LSB;
    LSB = ((data >> 8) & 0xFF); // The MSB of the data should be the LSB...
    MSB = (data & 0xFF); // ...and vice versa
    uint16_t swapped = (MSB << 8) | LSB; // Stitch the two bytes back together
    return swapped;
}

uint16_t CurrentSensors::readReg(){
    uint16_t read_data = 0;
    this->bus.read(this->ADS1015_read, (char*) &read_data, 2);
    return read_data;
}

bool CurrentSensors::writeAddrReg(ADS1015RegAddressPtr reg){
    uint8_t addressPtrReg = reg;
    return this->bus.write(this->ADS1015_write, (char*) &addressPtrReg, 1);
}
bool CurrentSensors::writeConfReg(ADS1015MuxConfigs muxConf){
    uint8_t write_data[3] = {0, 0, 0};
    
    // Set Address Pointer Register
    write_data[0] = confReg; // Tell ADS1015 we're writing to the configuration register
    
    // MSB of the Config Register
    write_data[1] |= (1 << 7); // Set OS bit
    write_data[1] |= (muxConf << 4); // Set multiplexer configuration
    write_data[1] |= (this->FSR << 1); // Set Full Scale Range

    // LSB of the Config Register
    write_data[2] |= (this->dataRate << 5); // Set data rate
    write_data[2] |= 0b11; // Disable comparator

    // Write all three bytes to the ADS1015
    return this->bus.write(this->ADS1015_write, (char*) &write_data, 3);
}

float CurrentSensors::readPDBCurrent(){
    // Set Full Scale Range to correct value for the PDB current sensor
    this->FSR = fsr2048;
    // Write to config register to get correct settings for reading
    this->writeConfReg(PDBCS);
    // First write to Address Pointer Register indicating we want to read the conversion register
    this->writeAddrReg(convReg);
    // Read the two bytes that are transmitted by the slave
    uint16_t read_data = this->readReg();
    // First swap the two bytes, because the LSB and MSB are received in the wrong order
    read_data = this->swapBytes(read_data);
    // Throw away the last four bits because they are always 0
    read_data = read_data >> 4;
    // Convert to a (float) voltage using the FSR
    float convertedVoltage = read_data * this->getLSBSize() / 1000; // In Volt
    // Convert from measured voltage to the actual current through the current sensor sensitivity and offset
    float convertedCurrent = (convertedVoltage * 1000 - this->ACS723sensorOffset) / this->ACS723sensorSensitivity; // In Ampere
    return convertedCurrent;
}

float CurrentSensors::readLV1Current(){
    // Set Full Scale Range to correct value for the PDB current sensor
    this->FSR = fsr2048;
    // Write to config register to get correct settings for reading
    this->writeConfReg(LVCS1);
    // First write to Address Pointer Register indicating we want to read the conversion register
    this->writeAddrReg(convReg);
    // Read the two bytes that are transmitted by the slave
    uint16_t read_data = this->readReg();
    // First swap the two bytes, because the LSB and MSB are received in the wrong order
    read_data = this->swapBytes(read_data);
    // Throw away the last four bits because they are always 0
    read_data = read_data >> 4;
    // Convert to a (float) voltage using the FSR
    float convertedVoltage = read_data * this->getLSBSize() / 1000; // In Volt
    // Convert from measured voltage to the actual current through the current sensor sensitivity and offset
    float convertedCurrent = (convertedVoltage * 1000 - this->ACS723sensorOffset) / this->ACS723sensorSensitivity; // In Ampere
    return convertedCurrent;
}

float CurrentSensors::readLV2Current(){
    return 0;
}

float CurrentSensors::readHVCurrent(){
    return 0;
}

