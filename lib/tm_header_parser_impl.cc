/* -*- c++ -*- */
/* 
 * Copyright 2017 <+YOU OR YOUR COMPANY+>.
 * 
 * This is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 * 
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this software; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <gnuradio/io_signature.h>
#include "tm_header_parser_impl.h"

extern "C"
{
    #include "telemetry/dslwp_tm_header.h"
}

#include <stdio.h>

using namespace boost::placeholders;

namespace gr {
  namespace dslwp {

    tm_header_parser::sptr
    tm_header_parser::make()
    {
      return gnuradio::get_initial_sptr
        (new tm_header_parser_impl());
    }

    /*
     * The private constructor
     */
    tm_header_parser_impl::tm_header_parser_impl()
      : gr::block("tm_header_parser",
              gr::io_signature::make(0, 0, sizeof(char)),
              gr::io_signature::make(0, 0, sizeof(char)))
    {
	d_in_port = pmt::mp("in");
      	message_port_register_in(d_in_port);

	d_out_port_0 = pmt::mp("out 0");	      
      	message_port_register_out(d_out_port_0);

	d_out_port_1 = pmt::mp("out 1");	      
      	message_port_register_out(d_out_port_1);

	d_out_port_2 = pmt::mp("out 2");	      
      	message_port_register_out(d_out_port_2);

	set_msg_handler(d_in_port, boost::bind(&tm_header_parser_impl::pmt_in_callback, this ,_1) );
    }

    /*
     * Our virtual destructor.
     */
    tm_header_parser_impl::~tm_header_parser_impl()
    {
    }

    void
    tm_header_parser_impl::forecast (int noutput_items, gr_vector_int &ninput_items_required)
    {
      /* <+forecast+> e.g. ninput_items_required[0] = noutput_items */
    }

    void tm_header_parser_impl::pmt_in_callback(pmt::pmt_t msg)
    {
	pmt::pmt_t meta(pmt::car(msg));
	pmt::pmt_t bytes(pmt::cdr(msg));

	unsigned int i;
	size_t msg_len;
	tm_header_t header;
	const uint8_t* bytes_in = pmt::u8vector_elements(bytes, msg_len);

	if(msg_len>=LEN_TM_HEADER)
	{
		((uint8_t *)&header)[0] = bytes_in[4];
		((uint8_t *)&header)[1] = bytes_in[3];
		((uint8_t *)&header)[2] = bytes_in[2];
		((uint8_t *)&header)[3] = bytes_in[1];
		((uint8_t *)&header)[4] = bytes_in[0];

		fprintf(stdout, "\n**** TM Frame Header\n");

		fprintf(stdout, "version_number = 0x%x\n", header.version_number);
		fprintf(stdout, "spacecraft_id = 0x%x\n", header.spacecraft_id);
		fprintf(stdout, "virtual_channel_id = 0x%x\n", header.virtual_channel_id);
		fprintf(stdout, "spare = 0x%x\n", header.spare);
		fprintf(stdout, "master_frame_count = %d\n", header.master_frame_count);
		fprintf(stdout, "virtual_channel_frame_count = %d\n", header.virtual_channel_frame_count);
		fprintf(stdout, "first_header_pointer = %d\n", header.first_header_pointer);

		switch(header.virtual_channel_id)
		{
			case 0:
			{
				tm_header_parser_impl::message_port_pub(tm_header_parser_impl::d_out_port_0, pmt::cons(pmt::make_dict(), pmt::init_u8vector(msg_len-LEN_TM_HEADER, (const uint8_t *)bytes_in+LEN_TM_HEADER)));
				break;
			}
			case 1:
			{
				tm_header_parser_impl::message_port_pub(tm_header_parser_impl::d_out_port_1, pmt::cons(pmt::make_dict(), pmt::init_u8vector(msg_len-LEN_TM_HEADER, (const uint8_t *)bytes_in+LEN_TM_HEADER)));
				break;
			}
			case 2:
			{
				tm_header_parser_impl::message_port_pub(tm_header_parser_impl::d_out_port_2, pmt::cons(pmt::make_dict(), pmt::init_u8vector(msg_len-LEN_TM_HEADER, (const uint8_t *)bytes_in+LEN_TM_HEADER)));
				break;
			}
		}
	}
	else
	{
		fprintf(stdout, "\n**** TM Frame Error: Too short PDU\n");
	}
    }

    int
    tm_header_parser_impl::general_work (int noutput_items,
                       gr_vector_int &ninput_items,
                       gr_vector_const_void_star &input_items,
                       gr_vector_void_star &output_items)
    {
      //const <+ITYPE+> *in = (const <+ITYPE+> *) input_items[0];
      //<+OTYPE+> *out = (<+OTYPE+> *) output_items[0];

      // Do <+signal processing+>
      // Tell runtime system how many input items we consumed on
      // each input stream.
      consume_each (noutput_items);

      // Tell runtime system how many output items we produced.
      return noutput_items;
    }

  } /* namespace dslwp */
} /* namespace gr */

