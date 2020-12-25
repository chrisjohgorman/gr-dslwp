/* -*- c++ -*- */
/* 
 * Copyright 2018 <+YOU OR YOUR COMPANY+>.
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
#include "ccsds_turbo_decode_impl.h"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define RATE_1_2 2
#define RATE_1_3 3
#define RATE_1_4 4
#define RATE_1_6 6

using namespace boost::placeholders;

namespace gr {
  namespace dslwp {

    ccsds_turbo_decode::sptr
    ccsds_turbo_decode::make(int base, int octets, int code_type, int iterations, float sigma, uint8_t update_sigma)
    {
      return gnuradio::get_initial_sptr
        (new ccsds_turbo_decode_impl(base, octets, code_type, iterations, sigma, update_sigma));
    }

    /*
     * The private constructor
     */
    ccsds_turbo_decode_impl::ccsds_turbo_decode_impl(int base, int octets, int code_type, int iterations, float sigma, uint8_t update_sigma)
      : gr::sync_block("ccsds_turbo_decode",
              gr::io_signature::make(0, 0, sizeof(int)),
              gr::io_signature::make(0, 0, sizeof(int))),
              d_base(base), d_octets(octets), d_code_type(code_type), d_iterations(iterations), d_sigma(sigma), d_update_sigma(update_sigma)
    {
		d_in_port = pmt::mp("in");
      	message_port_register_in(d_in_port);

		d_out_port = pmt::mp("out");	      
      	message_port_register_out(d_out_port);

		set_msg_handler(d_in_port, boost::bind(&ccsds_turbo_decode_impl::pmt_in_callback, this ,_1) );

		d_info_length = base * 8 * octets;

		int p[8] = {31, 37, 43, 47, 53, 59, 61, 67};
		int k1 = 8;
		int k2 = base * octets;

		d_pi = (int *)malloc(d_info_length * sizeof *d_pi);

		for (int s = 1; s <= d_info_length; ++s) {
			int m = (s-1) % 2;
			int i = (int) floor((s-1) / (2 * k2));
			int j = (int) floor((s-1) / 2) - i*k2;
			int t = (19*i + 1) % (k1/2);
			int q = t % 8 + 1;
			int c = (p[q-1]*j + 21*m) % k2;
			d_pi[s-1] = 2*(t + c*(k1/2) + 1) - m - 1;
		}

		int N_components_upper;
		int N_components_lower;

		d_backward = "0011";

		switch(d_code_type)
		{
			case RATE_1_2:
			{
				N_components_upper = 2;
				N_components_lower = 1;

				d_forward_upper[0] = "10011"; // systematic output
				d_forward_upper[1] = "11011";

				d_forward_lower[0] = "11011";
				// need to define puncturing pattern here maybe with a pointer to function
				// 110 101 110 101 110 101

				d_code1 = convcode_initialize((char **)d_forward_upper, (char *)d_backward, N_components_upper);
				d_code2 = convcode_initialize((char **)d_forward_lower, (char *)d_backward, N_components_lower);
				d_turbo = turbo_initialize(d_code1, d_code2, d_pi, d_info_length);
				d_rate = 1.0/2.0;
				d_encoded_length = d_turbo.encoded_length*2/3;
				break;
			}
			case RATE_1_3:
			{
				N_components_upper = 2;
				N_components_lower = 1;

				d_forward_upper[0] = "10011"; // systematic output
				d_forward_upper[1] = "11011";

				d_forward_lower[0] = "11011"; // no need for puncturing

				d_code1 = convcode_initialize((char **)d_forward_upper, (char *)d_backward, N_components_upper);
				d_code2 = convcode_initialize((char **)d_forward_lower, (char *)d_backward, N_components_lower);
				d_turbo = turbo_initialize(d_code1, d_code2, d_pi, d_info_length);
				d_rate = 1.0/3.0;
				d_encoded_length = d_turbo.encoded_length;
				break;
			}
			case RATE_1_4:
			{
				N_components_upper = 3;
				N_components_lower = 1;

				d_forward_upper[0] = "10011"; // systematic output
				d_forward_upper[1] = "10101";
				d_forward_upper[2] = "11111";

				d_forward_lower[0] = "11011"; // no need for puncturing

				d_code1 = convcode_initialize((char **)d_forward_upper, (char *)d_backward, N_components_upper);
				d_code2 = convcode_initialize((char **)d_forward_lower, (char *)d_backward, N_components_lower);
				d_turbo = turbo_initialize(d_code1, d_code2, d_pi, d_info_length);
				d_rate = 1.0/4.0;
				d_encoded_length = d_turbo.encoded_length;
				break;
			}
			case RATE_1_6:
			{
				N_components_upper = 4;
				N_components_lower = 2;

				d_forward_upper[0] = "10011"; // systematic output
				d_forward_upper[1] = "11011";
				d_forward_upper[2] = "10101";
				d_forward_upper[3] = "11111";

				d_forward_lower[0] = "11011"; // no need for puncturing
				d_forward_lower[1] = "11111";

				d_code1 = convcode_initialize((char **)d_forward_upper, (char *)d_backward, N_components_upper);
				d_code2 = convcode_initialize((char **)d_forward_lower, (char *)d_backward, N_components_lower);
				d_turbo = turbo_initialize(d_code1, d_code2, d_pi, d_info_length);
				d_rate = 1.0/6.0;
				d_encoded_length = d_turbo.encoded_length;
				break;
			}
			default:
			{
				fprintf(stdout, "\n**** ERROR: Unknown code type!\n");
				exit(0);
			}
		}

		fprintf(stdout, "\n**** Turbo decoder initiated:\n");
		fprintf(stdout, "rate = %f\n", d_rate);
		fprintf(stdout, "info_length = %d\n", d_info_length);
		fprintf(stdout, "encoded_length = %d\n", d_encoded_length);
    }

    /*
     * Our virtual destructor.
     */
    ccsds_turbo_decode_impl::~ccsds_turbo_decode_impl()
    {
    }

	// puncturing function: return 1 if bit k has to be punctured
	int ccsds_turbo_decode_impl::puncturing(int k){

		int bit_idx = k % 3;

		// bit 0,3,6,... corresponding to systematic output
		if (!bit_idx)
		    return 1;

		// get block index
		int block_idx = k / 3;

		// on odd blocks puncture second bit
		if (block_idx % 2){
		    return bit_idx != 1;
		}

		// on even blocks puncture third bit
		return bit_idx != 2;
	}

    void ccsds_turbo_decode_impl::pmt_in_callback(pmt::pmt_t msg)
    {
		pmt::pmt_t meta(pmt::car(msg));
		pmt::pmt_t bits(pmt::cdr(msg));

		if(d_update_sigma)
		{
			pmt::pmt_t p_value = pmt::dict_ref(meta, pmt::mp("eb_n0_est"), pmt::mp("not_found"));
			if(pmt::is_real(p_value))
			{
				float value = pmt::to_double(p_value);
				float sigma_old = d_sigma;
				d_sigma = sqrt(1.0f/(value*2.0f));
				fprintf(stdout, "\n**** Set sigma: %f -> %f\n", sigma_old, d_sigma);
			}
		}

		size_t msg_len;
		const float* bits_in = pmt::f32vector_elements(bits, msg_len);

		if(msg_len == d_encoded_length)
		{
			double *bits_depunctured = (double *)malloc(sizeof(double)*d_turbo.encoded_length);
			if(d_code_type == RATE_1_2)
			{
				int j=0;
				for(int i=0; i<d_turbo.encoded_length; i++)
				{
					if(puncturing(i))
					{
						bits_depunctured[i] = bits_in[j];
						j++;
					}
					else
					{
						bits_depunctured[i] = 0.0;
					}
				}
			}
			else
			{
				for(int i=0; i<d_encoded_length; i++)
				{
					bits_depunctured[i] = bits_in[i];
				}
			}

			int *decoded = turbo_decode(bits_depunctured, d_iterations, d_sigma*d_sigma, d_turbo);

			uint8_t *decoded_u8 = (uint8_t *)malloc(sizeof(uint8_t)*d_info_length/8);
			for(int i=0; i<d_info_length/8; i++)
			{
				decoded_u8[i] = 0;
				for(int j=0; j<8; j++)
				{
					decoded_u8[i] |= decoded[i*8+j]?(0x80>>j):0;
				}
			}

			ccsds_turbo_decode_impl::message_port_pub(ccsds_turbo_decode_impl::d_out_port, pmt::cons(pmt::make_dict(), pmt::init_u8vector(d_info_length/8, decoded_u8)));

			free(bits_depunctured);
			free(decoded);
			free(decoded_u8);
		}
		else
		{
			fprintf(stdout, "\n**** ERROR: Turbo decoder input length do not match!\n");
		}
    }

    int
    ccsds_turbo_decode_impl::work(int noutput_items,
        gr_vector_const_void_star &input_items,
        gr_vector_void_star &output_items)
    {
      // Tell runtime system how many output items we produced.
      return noutput_items;
    }

  } /* namespace dslwp */
} /* namespace gr */

