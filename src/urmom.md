
# Sensor Specifications

```sh
Invoke-WebRequest -Uri "https://seawall.fiu.edu/sensorSpecifications" `
  -Method POST `
  -Body '{"type":"float", "version":"1", "readValLabel":"humidity"}' `
  -ContentType "application/json"
```
return ...

```sh
Invoke-WebRequest -Uri "https://seawall.fiu.edu/sensorSpecifications" `
  -Method POST `
  -Body '{"type":"float", "version":"1", "readValLabel":"temperature"}' `
  -ContentType "application/json"
```
return ...

```sh
Invoke-WebRequest -Uri "https://seawall.fiu.edu/sensorSpecifications" `
  -Method POST `
  -Body '{"type":"float", "version":"1", "readValLabel":"turbidity"}' `
  -ContentType "application/json"
```
return ...

```sh
Invoke-WebRequest -Uri "https://seawall.fiu.edu/sensorSpecifications" `
  -Method POST `
  -Body '{"type":"float", "version":"1", "readValLabel":"salinity"}' `
  -ContentType "application/json"
```
return ...

```sh
Invoke-WebRequest -Uri "https://seawall.fiu.edu/sensorSpecifications" `
  -Method POST `
  -Body '{"type":"float", "version":"1", "readValLabel":"tds"}' `
  -ContentType "application/json"
```
return ...

```sh
Invoke-WebRequest -Uri "https://seawall.fiu.edu/sensorSpecifications" `
  -Method POST `
  -Body '{"type":"float", "version":"1", "readValLabel":"ec"}' `
  -ContentType "application/json"
```
return ...

```sh
Invoke-WebRequest -Uri "https://seawall.fiu.edu/sensorSpecifications" `
  -Method POST `
  -Body '{"type":"float", "version":"1", "readValLabel":"ph"}' `
  -ContentType "application/json"
```
return ...

```sh
Invoke-WebRequest -Uri "https://seawall.fiu.edu/sensorSpecifications" `
  -Method POST `
  -Body '{"type":"float", "version":"1", "readValLabel":"oygenLevel"}' `
  -ContentType "application/json"
```
return ...

# Sensor Array Specificaitons

```sh
Invoke-WebRequest -Uri "https://seawall.fiu.edu/sensorArraySpecifications" `
  -Method POST `
  -Body '{"type":"data", "version":"1"}' `
  -ContentType "application/json"
```
return ID = 1 
<!-- WE GOT IT -->

# BOXES BOXES BOXES
 <!-- already done JAJAJ -->

```sh
Invoke-WebRequest -Uri "https://seawall.fiu.edu/boxes" `
  -Method POST `
  -Body '{"nickname":"box1","public":"1"}' `
  -ContentType "application/json"
```
return ID = 2

# Sensor Array

```sh
Invoke-WebRequest -Uri "https://seawall.fiu.edu/sensorArrays" `
  -Method POST `
  -Body '{"sensor$array_specificationID":"1", "boxID":"2" }' `
  -ContentType "application/json"
```
return ...

# Sensors Declaration

```sh
Invoke-WebRequest -Uri "https://seawall.fiu.edu/sensorArrays" `
  -Method POST `
  -Body '{"sensor_specificationID":"INT", "sensor$arrayID":"1"' `
  -ContentType "application/json"
```
return ...

<!-- do for amount of sensors -->
<!-- Put into sensorID into ESP to send with post requests -->


# Sensor Reads
<!-- SUBMIT READ ON ESP !!!!!!!!!!!!!!!!!!!! -->





