# matOSC
subroutine MEX file for uint8 arrays of OSC message in MATLAB.

## How to Compile
`>> mex matOSC.c`

## How to Use
```
>> osc = udp('127.0.0.1', 8080);
>> fopen(osc)
>> fwrite(osc, matOSC('/matlab','test', 'ifis', -2, 3.141592, 1000, 'hello'))
```
