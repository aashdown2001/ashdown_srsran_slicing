/*
 *
 * Copyright 2013-2022 Software Radio Systems Limited
 *
 * By using this file, you agree to the terms and conditions set
 * forth in the LICENSE file which can be found at the top level of
 * the distribution.
 *
 */

#include "srsgnb/asn1/ngap/ngap.h"
#include "srsgnb/pcap/mac_pcap.h"
#include "srsgnb/pcap/ngap_pcap.h"
#include "srsgnb/support/test_utils.h"
#include <gtest/gtest.h>

class pcap_mac_test : public ::testing::Test
{
protected:
  void SetUp() override
  {
    srslog::fetch_basic_logger("PCAP").set_level(srslog::basic_levels::debug);
    srslog::fetch_basic_logger("PCAP").set_hex_dump_max_size(-1);

    test_logger.set_level(srslog::basic_levels::debug);
    test_logger.set_hex_dump_max_size(-1);

    // Start the log backend.
    srslog::init();

    // Start the PCAP
    mac_pcap_writer.open("mac.pcap");
  }

  void TearDown() override
  {
    mac_pcap_writer.close();
    // flush logger after each test
    srslog::flush();
  }

  srsgnb::mac_pcap      mac_pcap_writer;
  srslog::basic_logger& test_logger = srslog::fetch_basic_logger("TEST");
};

TEST_F(pcap_mac_test, write_pdu)
{
  // std::array<uint8_t, 11>     tv     = {0x42, 0x00, 0x08, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88};
  // std::array<uint8_t, 13>     tv     = {0x42, 0x00, 0x0a, 0x0d, 0x72, 0x80, 0xd3, 0x96, 0x02, 0x7b, 0x01, 0xbd,
  // 0x26};
  std::array<uint8_t, 17> tv = {
      0x04, 0x0a, 0x0d, 0x72, 0x80, 0xd3, 0x96, 0x02, 0x7b, 0x01, 0xbd, 0x26, 0x3f, 0x00, 0x00, 0x00, 0x00};
  int                         crnti  = 0x01011;
  int                         ue_id  = 2;
  int                         harqid = 0;
  int                         tti    = 10;
  srsgnb::mac_nr_context_info context;
  context.radioType           = srsgnb::PCAP_FDD_RADIO;
  context.direction           = srsgnb::PCAP_DIRECTION_DOWNLINK;
  context.rntiType            = srsgnb::PCAP_C_RNTI;
  context.rnti                = crnti;
  context.ueid                = ue_id;
  context.harqid              = harqid;
  context.system_frame_number = tti / 10;
  context.sub_frame_number    = tti % 10;
  mac_pcap_writer.push_pdu(context, tv);
  // ngap_pcap_writer.write_pdu(tv);
}