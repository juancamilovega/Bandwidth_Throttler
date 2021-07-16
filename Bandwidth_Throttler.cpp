#include "hls_stream.h"
#include "ap_int.h"
#include "ap_utils.h"

//Parameters

#define BYTE_WIDTH_OF_DATA 64
#define width_of_penalty 32
#define Scaling_Factor 1000
//  select width_of_penalty such that 2^width_of_penalty > max_penalty * max_packet_size / DW
//Calculated pre-processed values
#define bitwidth (BYTE_WIDTH_OF_DATA * 8)


struct net_data
{
	ap_uint<bitwidth> data;
	ap_uint<BYTE_WIDTH_OF_DATA> keep;
	ap_uint<1> last;
	//Add any other necessary sideband signals
	//ap_uint<16> dest;
};

void bandwidth_throttler(
	hls::stream<net_data> &data_in,
	hls::stream<net_data> &data_out,
	ap_uint<width_of_penalty> penalty
)
{
#pragma HLS DATAFLOW
#pragma HLS INTERFACE ap_ctrl_none port=return
#pragma HLS resource core=AXI4Stream variable = data_in
#pragma HLS DATA_PACK variable=data_in
#pragma HLS resource core=AXI4Stream variable = data_out
#pragma HLS DATA_PACK variable=data_out
	net_data temp_data;
	static ap_uint<width_of_penalty> accumulated_penalty = 0;
	static ap_uint<1> active = 0;
	if (((active == 1) || (accumulated_penalty<Scaling_Factor)) && !data_in.empty() && !data_out.full())
	{
		temp_data = data_in.read();
		accumulated_penalty = accumulated_penalty + penalty;
		active = temp_data.last == 1 ? 0 : 1;
		data_out.write(temp_data);
	}
	else
	{
		accumulated_penalty = (accumulated_penalty > Scaling_Factor) ? (ap_uint<width_of_penalty> )(accumulated_penalty - Scaling_Factor) : (ap_uint<width_of_penalty> )(0);
	}
}
