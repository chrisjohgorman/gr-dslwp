# Copyright 2011 Free Software Foundation, Inc.
#
# This file is part of GNU Radio
#
# GNU Radio is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3, or (at your option)
# any later version.
#
# GNU Radio is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with GNU Radio; see the file COPYING.  If not, write to
# the Free Software Foundation, Inc., 51 Franklin Street,
# Boston, MA 02110-1301, USA.
install(FILES
    dslwp_tm_header_parser.block.yml
    dslwp_tm_parser.block.yml
    dslwp_tm_parser.block.yml
    dslwp_steam_to_vector_moving_window_cc.block.yml
    dslwp_vector_padding_cc.block.yml
    dslwp_fft_estimator_cc.block.yml
    dslwp_set_gain_by_tag_cc.block.yml
    dslwp_oqpsk_coherent_demod_cc.block.yml
    dslwp_frame_spliter_c.block.yml
    dslwp_ccsds_turbo_encode.block.yml
    dslwp_ccsds_turbo_decode.block.yml
    dslwp_attach_sync_marker.block.yml
    dslwp_ccsds_pseudo_randomizer.block.yml
    dslwp_kiss_encode_pdu.block.yml
    dslwp_kiss_decode_pdu.block.yml
    dslwp_fec_encode_b.block.yml
    dslwp_differential_phase_detection_1bit_cf.block.yml
    dslwp_frame_spliter_f.block.yml
    dslwp_program_tracking_cc.block.yml
    dslwp_lrtc_mod_bc.block.yml DESTINATION share/gnuradio/grc/blocks
)
