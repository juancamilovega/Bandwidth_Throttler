# Packet Generator and Bandwidth Throttler

This repository includes two cores. First is a packet generator that allows the user to generate packets at a specified rate. Second is a bandwidth throttler that allows you to selectively set a maximum bandwidth for an AXI Stream port. This employs backpressure to enforce the bitrate that crosses it is strictly less than the amount specified (and exactly equal if the output applies no backpressure and the input is always ready). Note the equations account for packet sizes that are not multiples of the axi stream data bus width.

## Behaviour of Cores

Rather than directly writing the bitrate to the fpga and wasting FPGA space to perform the math, this data needs to be pre-computed and converted into two tuning parameters, the Scaling Factor (SF) and the penalty (PE). The relationship of the FPGA core is as follows, every clock cycle the accomulation function, A(t) evolves as follows:

![image](https://user-images.githubusercontent.com/11798516/125960553-8b0906da-fed2-49a6-94f6-a6fa08f6f860.png)

Where we will send data if the recipient can accept data (tReady is set) and one of the following conditions is true:
  - A packet is in the middle of being sent or 
  - A(t) ≤ SF 

The first rule is to ensure that packets are bundled together and instead we use the space between packets to toggle the bitrate.

In the case of the bandwidth throttler we say that if the above conditions is true then we allow a packet to pass if one is available at the input. Otherwise data is not sent if there is nothing to send.

## Bitrate equations

### Packet Generator

As proven on the provided PDF, Given:
  -	Tuning parameters are the Penalty (PE) and Scaling Factor (SF) [default 1000]
  -	Width of the AXI Stream line is DW bits
  -	Packets sent are PS bits long (must be a multiple of 8 to be valid in AXI Stream)
  -	The header added after this core is HS bits long
  -	The clock rate is CR measured in MHz
  -	The bandwidth (BW) is given in Mbps

![image](https://user-images.githubusercontent.com/11798516/125961243-00f8f510-fb3e-4dc1-9188-6c4dcec339a9.png)

For the bandwidth throttler as explained in the proof a simpler equation can be used that doesn't include PS as it is not known in this configuration. The equation is:

![image](https://user-images.githubusercontent.com/11798516/125961445-e431f1fd-6c48-47e9-8e77-d3eda4792973.png)

Using these equations one can either calculate the penalty for a certain bandwidth or the bandwidth for a certain penalty. It is recommended that SF use a constant value.

## Setting the Packet Size

The Packet Generator allows for any packet size, not just multiples of the data bus width. The bitrate equations handle this. To program the board, for an DW bit bus for PS packet size, both in bits, set:

![image](https://user-images.githubusercontent.com/11798516/126000650-741902a7-cd79-4054-9683-de49966e6b4a.png)


Note the Last Keep equation is all 1s when PS is a multiple of DW, otherwise it is a series of N 1s followed by all 0s where N is the number of bytes to be transmitted in the last flit.

## HLS Cores

The Cores are built using Vivado HLS and were tested on version 2018.3. The packet generator uses 107 FFs and 305 LUTs in the maximum width configuration, and the throttler uses  35 FFs and 198 LUTs in the maimum width configuration. Both can operate correctly at clock rates up to 400MHz and bus widths up to 1024 bits wide allowing it to generate precise bitrates of up to 400Gbps traffic. 
