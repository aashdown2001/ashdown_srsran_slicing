/*
 *
 * Copyright 2013-2022 Software Radio Systems Limited
 *
 * By using this file, you agree to the terms and conditions set
 * forth in the LICENSE file which can be found at the top level of
 * the distribution.
 *
 */

#include "lib/e1_interface/common/e1ap_asn1_utils.h"
#include "srsgnb/e1_interface/cu_cp/e1_cu_cp.h"
#include "srsgnb/e1_interface/cu_cp/e1_cu_cp_factory.h"
#include "srsgnb/support/async/async_test_utils.h"
#include "srsgnb/support/test_utils.h"
#include "unittests/e1_interface/common/e1_cu_cp_test_helpers.h"
#include "unittests/e1_interface/common/test_helpers.h"
#include <gtest/gtest.h>

using namespace srsgnb;
using namespace srs_cu_cp;

///////////////////////////////////////
// CU-CP initiated E1 Setup Procedure
///////////////////////////////////////

/// Test successful cu-cp initiated e1 setup procedure
TEST_F(e1_cu_cp_test, when_e1_setup_response_received_then_cu_cp_connected)
{
  // Action 1: Launch E1 setup procedure
  cu_cp_e1_setup_request_message request_msg = generate_cu_cp_e1_setup_request_message();
  test_logger.info("Launch e1 setup request procedure...");
  async_task<cu_cp_e1_setup_response_message>         t = e1->handle_cu_cp_e1_setup_request(request_msg);
  lazy_task_launcher<cu_cp_e1_setup_response_message> t_launcher(t);

  // Status: CU-UP received E1 Setup Request.
  ASSERT_EQ(msg_notifier->last_e1_msg.pdu.type().value, asn1::e1ap::e1_ap_pdu_c::types_opts::init_msg);
  ASSERT_EQ(msg_notifier->last_e1_msg.pdu.init_msg().value.type().value,
            asn1::e1ap::e1_ap_elem_procs_o::init_msg_c::types_opts::gnb_cu_cp_e1_setup_request);

  // Status: Procedure not yet ready.
  ASSERT_FALSE(t.ready());

  // Action 2: E1 setup response received.
  unsigned   transaction_id    = get_transaction_id(msg_notifier->last_e1_msg.pdu).value();
  e1_message e1_setup_response = generate_cu_cp_e1_setup_respose(transaction_id);
  test_logger.info("Injecting CuCpE1SetupResponse");
  e1->handle_message(e1_setup_response);

  ASSERT_TRUE(t.ready());
  ASSERT_TRUE(t.get().success);
  ASSERT_EQ(t.get().response->gnb_cu_up_name.value.to_string(), "srsCU-UP");
}

/// Test unsuccessful f1 setup procedure with time to wait and successful retry
TEST_F(e1_cu_cp_test, when_e1_setup_failure_with_time_to_wait_received_then_retry_with_success)
{
  // Action 1: Launch E1 setup procedure
  cu_cp_e1_setup_request_message request_msg = generate_cu_cp_e1_setup_request_message();
  test_logger.info("Launch e1 setup request procedure...");
  async_task<cu_cp_e1_setup_response_message>         t = e1->handle_cu_cp_e1_setup_request(request_msg);
  lazy_task_launcher<cu_cp_e1_setup_response_message> t_launcher(t);

  // Status: CU-UP received E1 Setup Request.
  ASSERT_EQ(msg_notifier->last_e1_msg.pdu.type().value, asn1::e1ap::e1_ap_pdu_c::types_opts::init_msg);
  ASSERT_EQ(msg_notifier->last_e1_msg.pdu.init_msg().value.type().value,
            asn1::e1ap::e1_ap_elem_procs_o::init_msg_c::types_opts::gnb_cu_cp_e1_setup_request);

  // Status: Procedure not yet ready.
  ASSERT_FALSE(t.ready());

  // Action 2: E1 setup failure with time to wait received.
  unsigned   transaction_id = get_transaction_id(msg_notifier->last_e1_msg.pdu).value();
  e1_message e1_setup_failure =
      generate_cu_cp_e1_setup_failure_message_with_time_to_wait(transaction_id, asn1::e1ap::time_to_wait_opts::v10s);
  test_logger.info("Injecting CuCpE1SetupFailure with time to wait");
  msg_notifier->last_e1_msg = {};
  e1->handle_message(e1_setup_failure);

  // Status: CU does not receive new F1 Setup Request until time-to-wait has ended.
  for (unsigned msec_elapsed = 0; msec_elapsed < 10000; ++msec_elapsed) {
    ASSERT_FALSE(t.ready());
    ASSERT_EQ(msg_notifier->last_e1_msg.pdu.type().value, asn1::e1ap::e1_ap_pdu_c::types_opts::nulltype);

    this->timers.tick_all();
  }

  // Status: CU-UP received E1 Setup Request again.
  ASSERT_EQ(msg_notifier->last_e1_msg.pdu.type().value, asn1::e1ap::e1_ap_pdu_c::types_opts::init_msg);
  ASSERT_EQ(msg_notifier->last_e1_msg.pdu.init_msg().value.type().value,
            asn1::e1ap::e1_ap_elem_procs_o::init_msg_c::types_opts::gnb_cu_cp_e1_setup_request);

  unsigned transaction_id2 = get_transaction_id(msg_notifier->last_e1_msg.pdu).value();
  EXPECT_NE(transaction_id, transaction_id2);

  // Successful outcome after reinitiated E1 Setup
  e1_message e1_setup_response = generate_cu_cp_e1_setup_respose(transaction_id2);
  test_logger.info("Injecting CuCpE1SetupResponse");
  e1->handle_message(e1_setup_response);

  ASSERT_TRUE(t.ready());
  ASSERT_TRUE(t.get().success);
  ASSERT_EQ(t.get().response->gnb_cu_up_name.value.to_string(), "srsCU-UP");
}

/// Test unsuccessful e1 setup procedure with time to wait and unsuccessful retry
TEST_F(e1_cu_cp_test, when_e1_setup_failure_with_time_to_wait_received_then_retry_without_success)
{
  // Action 1: Launch E1 setup procedure
  cu_cp_e1_setup_request_message request_msg = generate_cu_cp_e1_setup_request_message();
  test_logger.info("Launch e1 setup request procedure...");
  async_task<cu_cp_e1_setup_response_message>         t = e1->handle_cu_cp_e1_setup_request(request_msg);
  lazy_task_launcher<cu_cp_e1_setup_response_message> t_launcher(t);

  // Status: CU received F1 Setup Request.
  ASSERT_EQ(msg_notifier->last_e1_msg.pdu.type().value, asn1::e1ap::e1_ap_pdu_c::types_opts::init_msg);
  ASSERT_EQ(msg_notifier->last_e1_msg.pdu.init_msg().value.type().value,
            asn1::e1ap::e1_ap_elem_procs_o::init_msg_c::types_opts::gnb_cu_cp_e1_setup_request);

  // Status: Procedure not yet ready.
  EXPECT_FALSE(t.ready());

  // Action 2: E1 setup failure with time to wait received.
  unsigned   transaction_id = get_transaction_id(msg_notifier->last_e1_msg.pdu).value();
  e1_message e1_setup_failure =
      generate_cu_cp_e1_setup_failure_message_with_time_to_wait(transaction_id, asn1::e1ap::time_to_wait_opts::v10s);
  test_logger.info("Injecting CuCpE1SetupFailure with time to wait");
  msg_notifier->last_e1_msg = {};
  e1->handle_message(e1_setup_failure);

  // Status: CU-UP does not receive new E1 Setup Request until time-to-wait has ended.
  for (unsigned msec_elapsed = 0; msec_elapsed < 10000; ++msec_elapsed) {
    ASSERT_FALSE(t.ready());
    ASSERT_EQ(msg_notifier->last_e1_msg.pdu.type().value, asn1::e1ap::e1_ap_pdu_c::types_opts::nulltype);

    this->timers.tick_all();
  }

  // Status: CU-UP received E1 Setup Request again.
  ASSERT_EQ(msg_notifier->last_e1_msg.pdu.type().value, asn1::e1ap::e1_ap_pdu_c::types_opts::init_msg);
  ASSERT_EQ(msg_notifier->last_e1_msg.pdu.init_msg().value.type().value,
            asn1::e1ap::e1_ap_elem_procs_o::init_msg_c::types_opts::gnb_cu_cp_e1_setup_request);

  unsigned transaction_id2 = get_transaction_id(msg_notifier->last_e1_msg.pdu).value();
  EXPECT_NE(transaction_id, transaction_id2);

  // Unsuccessful outcome after reinitiated E1 Setup
  e1_setup_failure = generate_cu_cp_e1_setup_failure_message(transaction_id2);
  test_logger.info("Injecting CuCpE1SetupFailure");
  e1->handle_message(e1_setup_failure);

  ASSERT_TRUE(t.ready());
  EXPECT_FALSE(t.get().success);
}

/// Test the e1 setup procedure
TEST_F(e1_cu_cp_test, when_retry_limit_reached_then_cu_cp_not_connected)
{
  // Action 1: Launch E1 setup procedure
  cu_cp_e1_setup_request_message request_msg = generate_cu_cp_e1_setup_request_message();
  test_logger.info("Launch e1 setup request procedure...");
  async_task<cu_cp_e1_setup_response_message>         t = e1->handle_cu_cp_e1_setup_request(request_msg);
  lazy_task_launcher<cu_cp_e1_setup_response_message> t_launcher(t);

  // Status: CU received F1 Setup Request.
  ASSERT_EQ(msg_notifier->last_e1_msg.pdu.type().value, asn1::e1ap::e1_ap_pdu_c::types_opts::init_msg);
  ASSERT_EQ(msg_notifier->last_e1_msg.pdu.init_msg().value.type().value,
            asn1::e1ap::e1_ap_elem_procs_o::init_msg_c::types_opts::gnb_cu_cp_e1_setup_request);

  // Status: Procedure not yet ready.
  EXPECT_FALSE(t.ready());

  for (unsigned i = 0; i < request_msg.max_setup_retries; i++) {
    // Status: F1 setup failure received.
    unsigned   transaction_id = get_transaction_id(msg_notifier->last_e1_msg.pdu).value();
    e1_message e1_setup_response_msg =
        generate_cu_cp_e1_setup_failure_message_with_time_to_wait(transaction_id, asn1::e1ap::time_to_wait_opts::v10s);
    msg_notifier->last_e1_msg = {};
    e1->handle_message(e1_setup_response_msg);

    // Status: CU-UP does not receive new E1 Setup Request until time-to-wait has ended.
    for (unsigned msec_elapsed = 0; msec_elapsed < 10000; ++msec_elapsed) {
      ASSERT_FALSE(t.ready());
      ASSERT_EQ(msg_notifier->last_e1_msg.pdu.type().value, asn1::e1ap::e1_ap_pdu_c::types_opts::nulltype);

      this->timers.tick_all();
    }
  }

  // Status: E1 setup failure received.
  unsigned   transaction_id = get_transaction_id(msg_notifier->last_e1_msg.pdu).value();
  e1_message e1_setup_response_msg =
      generate_cu_cp_e1_setup_failure_message_with_time_to_wait(transaction_id, asn1::e1ap::time_to_wait_opts::v10s);
  msg_notifier->last_e1_msg = {};
  e1->handle_message(e1_setup_response_msg);

  ASSERT_TRUE(t.ready());
  ASSERT_FALSE(t.get().success);
  ASSERT_EQ(msg_notifier->last_e1_msg.pdu.type().value, asn1::e1ap::e1_ap_pdu_c::types_opts::nulltype);
}

/////////////////////////////////////
// CU-UP initiated E1 Setup Procedure
/////////////////////////////////////

/// Test the successful CU-UP initiated e1 setup procedure
TEST_F(e1_cu_cp_test, when_received_e1_setup_request_valid_then_connect_cu_up)
{
  // Action 1: Receive CuUpE1SetupRequest message
  test_logger.info("TEST: Receive CuUpE1SetupRequest message...");

  // Generate E1SetupRequest
  e1_message e1_setup_msg = generate_valid_cu_up_e1_setup_request();
  e1->handle_message(e1_setup_msg);

  // Action 2: Check if CuUpE1SetupRequest was forwarded to DU processor
  ASSERT_EQ(du_processor_notifier->last_cu_up_e1_setup_request_msg.request->gnb_cu_up_name.value.to_string(),
            "srsCU-UP");

  // Action 3: Transmit CuUpE1SetupResponse message
  test_logger.info("TEST: Transmit CuUpE1SetupResponse message...");
  cu_up_e1_setup_response_message msg = {};
  msg.success                         = true;
  e1->handle_cu_up_e1_setup_response(msg);

  // Check the generated PDU is indeed the E1 Setup response
  ASSERT_EQ(asn1::e1ap::e1_ap_pdu_c::types_opts::options::successful_outcome, msg_notifier->last_e1_msg.pdu.type());
  ASSERT_EQ(asn1::e1ap::e1_ap_elem_procs_o::successful_outcome_c::types_opts::options::gnb_cu_up_e1_setup_resp,
            msg_notifier->last_e1_msg.pdu.successful_outcome().value.type());
}

/// Test the e1 setup failure
TEST_F(e1_cu_cp_test, when_received_e1_setup_request_invalid_then_reject_cu_up)
{
  // Generate CuUpE1SetupRequest
  e1_message e1_setup_msg         = generate_cu_up_e1_setup_request_base();
  auto&      setup_req            = e1_setup_msg.pdu.init_msg().value.gnb_cu_up_e1_setup_request();
  setup_req->supported_plmns.id   = ASN1_E1AP_ID_SUPPORTED_PLMNS;
  setup_req->supported_plmns.crit = asn1::crit_opts::reject;
  e1->handle_message(e1_setup_msg);

  // Action 2 : Check if E1SetupRequest was forwarded to DU processor
  ASSERT_EQ(du_processor_notifier->last_cu_up_e1_setup_request_msg.request->gnb_cu_up_name.value.to_string(),
            "srsCU-UP");

  // Action 3: Transmit E1SetupFailure message
  test_logger.info("TEST: Transmit CuUpE1SetupFailure message...");
  cu_up_e1_setup_response_message msg = {};
  msg.success                         = false;
  e1->handle_cu_up_e1_setup_response(msg);

  // Check the generated PDU is indeed the E1 Setup failure
  ASSERT_EQ(asn1::e1ap::e1_ap_pdu_c::types_opts::options::unsuccessful_outcome, msg_notifier->last_e1_msg.pdu.type());
  ASSERT_EQ(asn1::e1ap::e1_ap_elem_procs_o::unsuccessful_outcome_c::types_opts::gnb_cu_up_e1_setup_fail,
            msg_notifier->last_e1_msg.pdu.unsuccessful_outcome().value.type());
}