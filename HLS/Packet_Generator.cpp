#include "hls_stream.h"
#include "ap_int.h"
#include "ap_utils.h"

//Parameters
#define BYTE_WIDTH_OF_DATA 64
#define width_of_penalty 32 //N
#define SCALING_FACTOR 1000 //SF

//Calculated pre-processed values
#define bitwidth (BYTE_WIDTH_OF_DATA * 8)
#define FULL_KEEP 0xFFFFFFFFFFFFFFFFLLU


struct net_data
{
	ap_uint<bitwidth> data;
	ap_uint<BYTE_WIDTH_OF_DATA> keep;
	ap_uint<1> last;
	//Add any other necessary sideband signals
	//ap_uint<16> dest;
	//ap_uint<16> id;
	//ap_uint<32> user;
};

void packet_generator(
	hls::stream<net_data> &data_out,
	ap_uint<width_of_penalty> penalty,
	ap_uint<1> run,
	ap_uint<8> num_flits_minus_1,
	ap_uint<64> last_keep,
	ap_uint<64> &packet_count
)
{
#pragma HLS DATAFLOW

#pragma HLS INTERFACE ap_ctrl_none port=return
#pragma HLS resource core=AXI4Stream variable = data_out
#pragma HLS DATA_PACK variable=data_out
	//Prepare the constants
	net_data temp_data;
	static ap_uint<8> flits_count=0; //How many flits of the current packet have been sent
	static ap_uint<64> packet_count_reg=0; //How many packets have been sent
	packet_count = packet_count_reg;
	static ap_uint<width_of_penalty> accumulated_penalty = 0; //A(t)
	static ap_uint<1> active = 0; //Indicates we are in the middle of sending a packet
	//Setup the data to send, use lower 64 bits to enumerate the packets
	temp_data.data.range(63,0) = packet_count_reg;
	temp_data.data.range(127,64) = 0xFEDBEEF1;
	temp_data.data.range(191,128) = 0xFEDBEEF2;
	temp_data.data.range(255,192) = 0xFEDBEEF3;
	temp_data.data.range(319,256) = 0xFEDBEEF4;
	temp_data.data.range(383,320) = 0xFEDBEEF5;
	temp_data.data.range(447,384) = 0xFEDBEEF6;
	temp_data.data.range(511,448) = 0xFEDBEEF7;
	if (((active == 1) || (accumulated_penalty<SCALING_FACTOR)) && !data_out.full())
	{
   		//We are sending a packet, increase A(t)
		accumulated_penalty = accumulated_penalty + penalty;
		if (flits_count>=num_flits_minus_1)
		{
			//End of a packet, active will go low and set the .last and .keep to match packet end
			active = 0;
			temp_data.last = 1;
			temp_data.keep = last_keep;
			//Reset the count of flits
			flits_count = 0;
			//Increment the packet index
			packet_count_reg = packet_count_reg + 1;
		}
		else
		{
			//There is more data to send, set or keep active at 1
			active = 1;
			//Indicate it is not the end using .keep and .last
			temp_data.last = 0;
			temp_data.keep = FULL_KEEP;
			//Increment the count of flits
			flits_count = flits_count + 1;
		}
		//Write the data out
		data_out.write(temp_data);
	}
	else
	{
		//Packet not sent so update A(t) to the max between 0 or a(t)-SF
		accumulated_penalty = (accumulated_penalty < SCALING_FACTOR) ? (ap_uint<width_of_penalty> )(0) : (ap_uint<width_of_penalty> )(accumulated_penalty - SCALING_FACTOR);
	}

}
