/*
mediastreamer2 library - modular sound and video processing and streaming
Copyright (C) 2006-2013 Belledonne Communications, Grenoble

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include "CUnit/Basic.h"
#include "mediastreamer2_tester_private.h"

#include "mediastreamer2/dtmfgen.h"
#include "mediastreamer2/msfileplayer.h"
#include "mediastreamer2/msfilerec.h"
#include "mediastreamer2/msrtp.h"
#include "mediastreamer2/mssndcard.h"
#include "mediastreamer2/mstonedetector.h"
#include "mediastreamer2/mswebcam.h"


typedef struct {
	MSDtmfGenCustomTone generated_tone;
	MSToneDetectorDef expected_tone;
} tone_test_def_t;


static tone_test_def_t tone_definition[] = {
	{ { "", 400, {2000,0}, 0.6f, 0 }, { "", 2000, 300, 0.5f } },
	{ { "", 600, {1500,0}, 1.0f, 0 }, { "", 1500, 500, 0.9f } },
	{ { "", 500,  {941,0}, 0.8f, 0 }, { "",  941, 400, 0.7f } }
};


MSTicker *ms_tester_ticker = NULL;
MSFilter *ms_tester_fileplay = NULL;
MSFilter *ms_tester_filerec = NULL;
MSFilter *ms_tester_dtmfgen = NULL;
MSFilter *ms_tester_tonedet = NULL;
MSFilter *ms_tester_voidsource = NULL;
MSFilter *ms_tester_voidsink = NULL;
MSFilter *ms_tester_encoder = NULL;
MSFilter *ms_tester_decoder = NULL;
MSFilter *ms_tester_rtprecv = NULL;
MSFilter *ms_tester_rtpsend = NULL;
MSFilter *ms_tester_resampler = NULL;
MSFilter *ms_tester_soundwrite = NULL;
MSFilter *ms_tester_soundread = NULL;
MSFilter *ms_tester_videocapture = NULL;
char *ms_tester_codec_mime = "pcmu";
unsigned char ms_tester_tone_detected;


static MSTicker * create_ticker(void) {
	MSTickerParams params;
	params.name = "Tester MSTicker";
	params.prio = MS_TICKER_PRIO_NORMAL;
	return ms_ticker_new_with_params(&params);
}


void ms_tester_create_ticker(void) {
	CU_ASSERT_PTR_NULL(ms_tester_ticker);
	ms_tester_ticker = create_ticker();
	CU_ASSERT_PTR_NOT_NULL_FATAL(ms_tester_ticker);
}

void ms_tester_destroy_ticker(void) {
	if (ms_tester_ticker) {
		ms_ticker_destroy(ms_tester_ticker);
		ms_tester_ticker = NULL;
	}
}

#define CREATE_FILTER(mask, filter, id) \
	if (filter_mask & mask) { \
		CU_ASSERT_PTR_NULL(filter); \
		filter = ms_filter_new(id); \
		CU_ASSERT_PTR_NOT_NULL_FATAL(filter); \
	}

void ms_tester_create_filter(MSFilter **filter, MSFilterId id) {
	CU_ASSERT_PTR_NOT_NULL(filter);
	CU_ASSERT_PTR_NULL(*filter);
	*filter = ms_filter_new(id);
	CU_ASSERT_PTR_NOT_NULL_FATAL(*filter);
}

void ms_tester_create_filters(unsigned int filter_mask) {
	MSSndCardManager *snd_manager;
	MSWebCamManager *cam_manager;
	MSSndCard *playcard;
	MSSndCard *captcard;
	MSWebCam *camera;

	CREATE_FILTER(FILTER_MASK_FILEPLAY, ms_tester_fileplay, MS_FILE_PLAYER_ID);
	CREATE_FILTER(FILTER_MASK_FILEREC, ms_tester_filerec, MS_FILE_REC_ID);
	CREATE_FILTER(FILTER_MASK_DTMFGEN, ms_tester_dtmfgen, MS_DTMF_GEN_ID);
	CREATE_FILTER(FILTER_MASK_TONEDET, ms_tester_tonedet, MS_TONE_DETECTOR_ID);
	CREATE_FILTER(FILTER_MASK_VOIDSOURCE, ms_tester_voidsource, MS_VOID_SOURCE_ID);
	CREATE_FILTER(FILTER_MASK_VOIDSINK, ms_tester_voidsink, MS_VOID_SINK_ID);
	if (filter_mask & FILTER_MASK_ENCODER) {
		CU_ASSERT_PTR_NULL(ms_tester_encoder);
		ms_tester_encoder = ms_filter_create_encoder(ms_tester_codec_mime);
		CU_ASSERT_PTR_NOT_NULL_FATAL(ms_tester_encoder);
	}
	if (filter_mask & FILTER_MASK_DECODER) {
		CU_ASSERT_PTR_NULL(ms_tester_decoder);
		ms_tester_decoder = ms_filter_create_decoder(ms_tester_codec_mime);
		CU_ASSERT_PTR_NOT_NULL_FATAL(ms_tester_decoder);
	}
	CREATE_FILTER(FILTER_MASK_RTPRECV, ms_tester_rtprecv, MS_RTP_RECV_ID);
	CREATE_FILTER(FILTER_MASK_RTPSEND, ms_tester_rtpsend, MS_RTP_SEND_ID);
	CREATE_FILTER(FILTER_MASK_RESAMPLER, ms_tester_resampler, MS_RESAMPLE_ID);
	if (filter_mask & FILTER_MASK_SOUNDWRITE) {
		CU_ASSERT_PTR_NULL(ms_tester_soundwrite);
		snd_manager = ms_snd_card_manager_get();
		playcard = ms_snd_card_manager_get_default_playback_card(snd_manager);
		CU_ASSERT_PTR_NOT_NULL_FATAL(playcard);
		ms_tester_soundwrite = ms_snd_card_create_writer(playcard);
		CU_ASSERT_PTR_NOT_NULL_FATAL(ms_tester_soundwrite);
	}
	if (filter_mask & FILTER_MASK_SOUNDREAD) {
		CU_ASSERT_PTR_NULL(ms_tester_soundread);
		snd_manager = ms_snd_card_manager_get();
		captcard = ms_snd_card_manager_get_default_capture_card(snd_manager);
		CU_ASSERT_PTR_NOT_NULL_FATAL(captcard);
		ms_tester_soundread = ms_snd_card_create_reader(captcard);
		CU_ASSERT_PTR_NOT_NULL_FATAL(ms_tester_soundread);
	}
	if (filter_mask & FILTER_MASK_VIDEOCAPTURE) {
		CU_ASSERT_PTR_NULL(ms_tester_videocapture);
		cam_manager = ms_web_cam_manager_get();
		camera = ms_web_cam_manager_get_default_cam(cam_manager);
		CU_ASSERT_PTR_NOT_NULL_FATAL(camera);
		ms_tester_videocapture = ms_web_cam_create_reader(camera);
		CU_ASSERT_PTR_NOT_NULL_FATAL(ms_tester_videocapture);
	}
}

#define DESTROY_FILTER(mask, filter) \
	if ((filter_mask & mask) & (filter != NULL)) { \
		ms_filter_destroy(filter); \
	} \
	filter = NULL

void ms_tester_destroy_filter(MSFilter **filter) {
	CU_ASSERT_PTR_NOT_NULL(filter);
	if (*filter != NULL) {
		ms_filter_destroy(*filter);
		*filter = NULL;
	}
}

void ms_tester_destroy_filters(unsigned int filter_mask) {
	DESTROY_FILTER(FILTER_MASK_FILEPLAY, ms_tester_fileplay);
	DESTROY_FILTER(FILTER_MASK_FILEREC, ms_tester_filerec);
	DESTROY_FILTER(FILTER_MASK_DTMFGEN, ms_tester_dtmfgen);
	DESTROY_FILTER(FILTER_MASK_TONEDET, ms_tester_tonedet);
	DESTROY_FILTER(FILTER_MASK_VOIDSOURCE, ms_tester_voidsource);
	DESTROY_FILTER(FILTER_MASK_VOIDSINK, ms_tester_voidsink);
	DESTROY_FILTER(FILTER_MASK_ENCODER, ms_tester_encoder);
	DESTROY_FILTER(FILTER_MASK_DECODER, ms_tester_decoder);
	DESTROY_FILTER(FILTER_MASK_RTPRECV, ms_tester_rtprecv);
	DESTROY_FILTER(FILTER_MASK_RTPSEND, ms_tester_rtpsend);
	DESTROY_FILTER(FILTER_MASK_RESAMPLER, ms_tester_resampler);
	DESTROY_FILTER(FILTER_MASK_SOUNDWRITE, ms_tester_soundwrite);
	DESTROY_FILTER(FILTER_MASK_SOUNDREAD, ms_tester_soundread);
	DESTROY_FILTER(FILTER_MASK_VIDEOCAPTURE, ms_tester_videocapture);
}

void ms_tester_tone_generation_loop(void) {
	unsigned int i;

	for (i = 0; i < (sizeof(tone_definition) / sizeof(tone_definition[0])); i++) {
		ms_filter_call_method(ms_tester_dtmfgen, MS_DTMF_GEN_PLAY_CUSTOM, &tone_definition[i].generated_tone);
		ms_sleep(1);
	}
}

void ms_tester_tone_detection_loop(void) {
	unsigned int i;

	for (i = 0; i < (sizeof(tone_definition) / sizeof(tone_definition[0])); i++) {
		ms_tester_tone_detected = FALSE;
		ms_filter_call_method(ms_tester_tonedet, MS_TONE_DETECTOR_CLEAR_SCANS, NULL);
		ms_filter_call_method(ms_tester_tonedet, MS_TONE_DETECTOR_ADD_SCAN, &tone_definition[i].expected_tone);
		ms_sleep(1);
		CU_ASSERT_EQUAL(ms_tester_tone_detected, TRUE);
	}
}

void ms_tester_tone_generation_and_detection_loop(void) {
	unsigned int i;

	for (i = 0; i < (sizeof(tone_definition) / sizeof(tone_definition[0])); i++) {
		ms_tester_tone_detected = FALSE;
		CU_ASSERT_EQUAL(0,ms_filter_call_method(ms_tester_tonedet, MS_TONE_DETECTOR_CLEAR_SCANS, NULL));
		CU_ASSERT_EQUAL(0,ms_filter_call_method(ms_tester_tonedet, MS_TONE_DETECTOR_ADD_SCAN, &tone_definition[i].expected_tone));
		CU_ASSERT_EQUAL(0,ms_filter_call_method(ms_tester_dtmfgen, MS_DTMF_GEN_PLAY_CUSTOM, &tone_definition[i].generated_tone));
		ms_sleep(1);
		CU_ASSERT_EQUAL(ms_tester_tone_detected, TRUE);
	}
}

bool_t wait_for_list(MSList *mss, int *counter, int value, int timeout_ms) {
	return wait_for_list_with_parse_events(mss, counter, value, timeout_ms, NULL, NULL);
}

bool_t wait_for_list_with_parse_events(MSList *mss, int *counter, int value, int timeout_ms, MSList *cbs, MSList *ptrs) {
	MSList *msi;
	MSList *cbi;
	MSList *ptri;
	int retry = 0;

	while ((*counter < value) && (retry++ < (timeout_ms / 100))) {
		for (msi = mss, cbi = cbs, ptri = ptrs; msi != NULL; msi = msi->next) {
			MediaStream *stream = (MediaStream *)msi->data;
			ms_tester_iterate_cb cb = NULL;
			media_stream_iterate(stream);
			if ((retry % 10) == 0) {
				ms_message("stream [%p] bandwidth usage: [d=%.1f,u=%.1f] kbit/sec",
					stream,
					media_stream_get_down_bw(stream) / 1000,
					media_stream_get_up_bw(stream) / 1000);
			}
			if (cbi && ptri) {
				cb = (ms_tester_iterate_cb)cbi->data;
				cb(stream, ptri->data);
			}
			if (cbi) cbi = cbi->next;
			if (ptri) ptri = ptri->next;
		}
		ms_usleep(100000);
	}

	if (*counter < value)
		return FALSE;
	return TRUE;
}

bool_t wait_for_until(MediaStream *ms_1, MediaStream *ms_2, int *counter, int value, int timeout_ms) {
	return wait_for_until_with_parse_events(ms_1, ms_2, counter, value, timeout_ms, NULL, NULL, NULL, NULL);
}

bool_t wait_for_until_with_parse_events(MediaStream *ms1, MediaStream *ms2, int *counter, int value, int timeout_ms, ms_tester_iterate_cb cb1, void *ptr1, ms_tester_iterate_cb cb2, void *ptr2) {
	MSList *mss = NULL;
	MSList *cbs = NULL;
	MSList *ptrs = NULL;
	bool_t result;

	if (ms1) {
		mss = ms_list_append(mss, ms1);
		if (cb1 && ptr1) {
			cbs = ms_list_append(cbs, cb1);
			ptrs = ms_list_append(ptrs, ptr1);
		}
	}
	if (ms2) {
		mss = ms_list_append(mss, ms2);
		if (cb2 && ptr2) {
			cbs = ms_list_append(cbs, cb2);
			ptrs = ms_list_append(ptrs, ptr2);
		}
	}
	result = wait_for_list_with_parse_events(mss, counter, value, timeout_ms, cbs, ptrs);
	ms_list_free(mss);
	ms_list_free(cbs);
	ms_list_free(ptrs);
	return result;
}

bool_t wait_for(MediaStream* ms_1, MediaStream* ms_2, int *counter, int value) {
	return wait_for_until(ms_1, ms_2, counter, value, 2000);
}
