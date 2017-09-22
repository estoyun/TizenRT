/* ****************************************************************
 *
 * Copyright 2017 Samsung Electronics All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 ******************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <signal.h>
#include <st_things/st_things.h>
#include <iotbus/iotbus_gpio.h>

#define TAG		"LIGHT_THINGS"

static const char *g_res_switch = "/switch";

extern bool handle_get_request_on_switch(st_things_get_request_message_s * req_msg, st_things_representation_s * resp_rep);
extern bool handle_set_request_on_switch(st_things_set_request_message_s * req_msg, st_things_representation_s * resp_rep);

#ifdef CONFIG_RESET_BUTTON
static bool check_reset_button_pin_number(void)
{
	printf("Current pin number : %d\n", CONFIG_RESET_BUTTON_PIN_NUMBER);
	return CONFIG_RESET_BUTTON_PIN_NUMBER >= 0 ? true : false;
}

static void gpio_callback_event(void *user_data)
{
	printf("gpio_callback_event!!\n");
	printf("reset :: %d\n", st_things_reset());
}
#endif

static void handle_reset_result(bool result)
{
	printf("[%s]Reset %s.\n", result ? "succeeded" : "failed", TAG);
}

static void handle_things_status_change(st_things_status_e things_status)
{
	printf("[%s]Things status is changed: %d\n", TAG, things_status);
}

static bool handle_reset_request(void)
{
	printf("[%s]Received a request for RESET.\n", TAG);
	return true;
}

static bool handle_ownership_transfer_request(void)
{
	printf("[%s]Received a request for Ownership-transfer. \n", TAG);
	return true;
}

static bool handle_get_request(st_things_get_request_message_s * req_msg, st_things_representation_s * resp_rep)
{
	bool ret = false;

	printf("Received a GET request on %s\n", req_msg->resource_uri);

	if (0 == strncmp(req_msg->resource_uri, g_res_switch, strlen(g_res_switch))) {
		ret = handle_get_request_on_switch(req_msg, resp_rep);
	} else {
		printf("Not supported uri.\n");
	}

	return ret;
}

static bool handle_set_request(st_things_set_request_message_s * req_msg, st_things_representation_s * resp_rep)
{
	printf("Received a SET request on %s\n", req_msg->resource_uri);

	if (0 == strncmp(req_msg->resource_uri, g_res_switch, strlen(g_res_switch))) {
		handle_set_request_on_switch(req_msg, resp_rep);
		return true;
	} else {
		return false;
	}

	return true;
}

int ess_process(void)
{
#ifdef CONFIG_RESET_BUTTON
	if (!check_reset_button_pin_number()) {
		printf("Error : Invalid pin number.\n");
		return 0;
	}

	iotbus_gpio_context_h m_gpio = iotbus_gpio_open(CONFIG_RESET_BUTTON_PIN_NUMBER);
	iotbus_gpio_register_cb(m_gpio, IOTBUS_GPIO_EDGE_RISING, gpio_callback_event, (void *)m_gpio);
#endif

	bool easysetup_complete = false;
	st_things_initialize("/rom/sampleDevice.json", &easysetup_complete);
	iotapi_initialize();

	st_things_register_request_cb(handle_get_request, handle_set_request);
	st_things_register_reset_cb(handle_reset_request, handle_reset_result);
	st_things_register_user_confirm_cb(handle_ownership_transfer_request);
	st_things_register_things_status_change_cb(handle_things_status_change);

	st_things_start();

	printf("[%s]=====================================================\n", TAG);
	printf("[%s]                    Stack Started                    \n", TAG);
	printf("[%s]=====================================================\n", TAG);

	return 0;
}
