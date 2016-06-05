#include <gtest/gtest.h>
#include <stdint.h>
#include <algorithm>
#include <vector>
#include <fstream>
#include "chromaprint.h"
#include "test_utils.h"
#include "utils/scope_exit.h"

namespace chromaprint {

TEST(API, TestFp) {
	std::vector<short> data = LoadAudioFile("data/test_stereo_44100.raw");

	ChromaprintContext *ctx = chromaprint_new(CHROMAPRINT_ALGORITHM_TEST2);
	ASSERT_NE(nullptr, ctx);
	SCOPE_EXIT(chromaprint_free(ctx));

	ASSERT_EQ(1, chromaprint_start(ctx, 44100, 1));
	ASSERT_EQ(1, chromaprint_feed(ctx, data.data(), data.size()));

	char *fp;
	uint32_t fp_hash;

	ASSERT_EQ(1, chromaprint_finish(ctx));
	ASSERT_EQ(1, chromaprint_get_fingerprint(ctx, &fp));
	SCOPE_EXIT(chromaprint_dealloc(fp));
	ASSERT_EQ(1, chromaprint_get_fingerprint_hash(ctx, &fp_hash));

	EXPECT_EQ(std::string("AQAAC0kkZUqYREkUnFAXHk8uuMZl6EfO4zu-4ABKFGESWIIMEQE"), std::string(fp));
	ASSERT_EQ(3732003127, fp_hash);
}

TEST(API, Test2SilenceFp)
{
	short zeroes[1024];
	std::fill(zeroes, zeroes + 1024, 0);

	ChromaprintContext *ctx = chromaprint_new(CHROMAPRINT_ALGORITHM_TEST2);
	ASSERT_NE(nullptr, ctx);
	SCOPE_EXIT(chromaprint_free(ctx));

	ASSERT_EQ(1, chromaprint_start(ctx, 44100, 1));
	for (int i = 0; i < 130; i++) {
		ASSERT_EQ(1, chromaprint_feed(ctx, zeroes, 1024));
	}

	char *fp;
	uint32_t fp_hash;

	ASSERT_EQ(1, chromaprint_finish(ctx));
	ASSERT_EQ(1, chromaprint_get_fingerprint(ctx, &fp));
	SCOPE_EXIT(chromaprint_dealloc(fp));
	ASSERT_EQ(1, chromaprint_get_fingerprint_hash(ctx, &fp_hash));

	ASSERT_EQ(18, strlen(fp));
	EXPECT_EQ(std::string("AQAAA0mUaEkSRZEGAA"), std::string(fp));
	ASSERT_EQ(627964279, fp_hash);
}

TEST(API, Test2SilenceRawFp)
{
	short zeroes[1024];
	std::fill(zeroes, zeroes + 1024, 0);

	ChromaprintContext *ctx = chromaprint_new(CHROMAPRINT_ALGORITHM_TEST2);
	ASSERT_NE(nullptr, ctx);
	SCOPE_EXIT(chromaprint_free(ctx));

	ASSERT_EQ(1, chromaprint_start(ctx, 44100, 1));
	for (int i = 0; i < 130; i++) {
		ASSERT_EQ(1, chromaprint_feed(ctx, zeroes, 1024));
	}

	uint32_t *fp;
	int length;

	ASSERT_EQ(1, chromaprint_finish(ctx));
	ASSERT_EQ(1, chromaprint_get_raw_fingerprint(ctx, &fp, &length));
	SCOPE_EXIT(chromaprint_dealloc(fp));

	ASSERT_EQ(3, length);
	EXPECT_EQ(627964279, fp[0]);
	EXPECT_EQ(627964279, fp[1]);
	EXPECT_EQ(627964279, fp[2]);
}

TEST(API, TestEncodeFingerprint)
{
	uint32_t fingerprint[] = { 1, 0 };
	char expected[] = { 55, 0, 0, 2, 65, 0 };

	char *encoded;
	int encoded_size;
	ASSERT_EQ(1, chromaprint_encode_fingerprint(fingerprint, 2, 55, &encoded, &encoded_size, 0));
	SCOPE_EXIT(chromaprint_dealloc(encoded));

	ASSERT_EQ(6, encoded_size);
	for (int i = 0; i < encoded_size; i++) {
		ASSERT_EQ(expected[i], encoded[i]) << "Different at " << i;
	}
}

TEST(API, TestEncodeFingerprintBase64)
{
	uint32_t fingerprint[] = { 1, 0 };
	char expected[] = "NwAAAkEA";

	char *encoded;
	int encoded_size;
	ASSERT_EQ(1, chromaprint_encode_fingerprint(fingerprint, 2, 55, &encoded, &encoded_size, 1));
	SCOPE_EXIT(chromaprint_dealloc(encoded));

	ASSERT_EQ(8, encoded_size);
	ASSERT_STREQ(expected, encoded);
}

TEST(API, TestDecodeFingerprint)
{
	char data[] = { 55, 0, 0, 2, 65, 0 };

	uint32_t *fingerprint;
	int size;
	int algorithm;
	ASSERT_EQ(1, chromaprint_decode_fingerprint(data, 6, &fingerprint, &size, &algorithm, 0));
	SCOPE_EXIT(chromaprint_dealloc(fingerprint));

	ASSERT_EQ(2, size);
	ASSERT_EQ(55, algorithm);
	ASSERT_EQ(1, fingerprint[0]);
	ASSERT_EQ(0, fingerprint[1]);
}

TEST(API, TestHashFingerprint)
{
	uint32_t fingerprint[] = { 19681, 22345, 312312, 453425 };
    uint32_t hash;

    ASSERT_EQ(0, chromaprint_hash_fingerprint(NULL, 4, &hash));
    ASSERT_EQ(0, chromaprint_hash_fingerprint(fingerprint, -1, &hash));
    ASSERT_EQ(0, chromaprint_hash_fingerprint(fingerprint, 4, NULL));

    ASSERT_EQ(1, chromaprint_hash_fingerprint(fingerprint, 4, &hash));
    ASSERT_EQ(17249, hash);
}

TEST(API, TestMatcher)
{
	const char *fp_roar1 = "AQADtIqiKIqURFWQ5lJRyFpRqRGH9Ohf7Hng_bhCCn3gcj5Cucahh8R7hBap4c4j3A-ewuuEnegX4hfysKgeHeYMjUd_hHlOnMf0ZBRh9jiF_kdeQhbD4x2DMFVm4VPc4ym2McPhy6gcKfhy5JqUEZXaBj60q-gPbz5SKrlRUisuXAgJigwp6OKROhf4HM6N08ed9Th-pIyY4jtElDRxd0adhB18En2KGzpyCSf6w6wOn8eRQ0dKB8dHPFXRH0EfmDd6oI1xHh9xcDWu5TjxPvhJ-NB4XEdtw0y0b-gvhIeT5iT-IjvMxBhZPDAf9MOhq8I1G8-GHuYo_MOPf5CPI8cfnCV-HEe64_hx-ML3wTF01saLdDyeH--PZz-Onipu_EecI8mXD1eO9hTO47uKz_Dzox5u5KKg3ji6JAsL8UcPHj8a_siJIwePE8_h43jwH3_waPiFNMebFWlvtPeglwLzwz9-fPxwPOWhH0584j94PC_GLMTlQ3_gow2PC7fwiwjPEM5x4ReOvBB_XET6HjfO4jhMxD_wlIdjHYdFHzoRnzfMHcdzWnhM_BuOWrSF_NARXdWCJ1lmodLxB31O9EdzpbhyHF9JfDyY-COq50J_ZJJ4qA5y_HheeF9wrkEYLodiWYif5PBzPLiFZzkuCd8SESWXoHke5MozJJ744BqaHs-YHj6P_kEvNI-C6A2S78UTvLjYGM4f8BGeKfh1NIt0PDGeJnpw83iHMpyRlYf-w26M_MdH4z-OnIcW_bgkjUFZoMcNX3hx8cZ35C_M4z2i4z1SHu-CC2fRPzh-IQyzjdD4hEX04z36D02VHFcR-sGdC__x7_AxiWnwRIQeo3mKEOeHJxV-4vGEMElOXGKJQ2_hG3_w4zA-Bbrw4MBN-EPewwqDI40W6LXxp0U64b9wPBSL8xV6Ckd8JOlzXNoZPGk0lA-uyTiPH352hFZwWdDj4LqHD9rCeIJL4UwbFO2PCz7OWEQt4uiVw7yOeoP24_DCHQ8eVDwulOsDkeomlG2H23iOo__gSUcdxviDPpwOKvnBveiHOg22ZFAP7cKRC19SGceloNoFq1GOXIfwFT_gGw-OE_KNWEhxn3iK_8KhJuVhxNXxR_DxCL85HEcySyLyKCn-Y8cffEdTB3W0B8d_PER47CFUapxCzMqDTtvRJ8KUC1f44GkTtBENK4VyoN6Fh7FwSmjIBd2DH5oXHO1z4kevDU0WBz_eoHly1JSM5wp-Cg2P8gcvDZpTUijfZHi0PESlWwjzPeBx5sSTcGjmE51xXDdypoJ-Y-JTpEEVLRde5BZ-iM_xHT08icZ_WE5whD-6JFSKJlr24EVlbRJSQZO_IleNnhzhqpqHfsSPl81xfsORI_lx7xjTxMLzHT8e32hqJ0E__HiAXJn8QG_iBP2FJo5x3MajZQu0H82O6wjLI53wZA_OHhcu_MTHByaJ4zmc2oGOa0Yu4VpyoZzC47rwnQjZA2Il5UWekgnKJ_iWUUaXw3keVMd1jJG0lAgzoT9U4q-CLye-4U_R5MabC9Wi4kZznNBDwx8e_HCO42qHd_B-9BHecNB4pPGLXkf5M_CpHag-H49_nMeDQOcbRB-eMOlx5dhRX3gaB_eM5sdzYJapB_miBk29QhN1Q5fA50Mb9fBxpGeK9_hB0biQp_DBQzQTfPjhH38VeBcqDvmJO_B6PB_eGDxC_9CjC_-O_BV4En8-kD_OC28mXMpUwhemH3nw3nggf8JNNHqELsV7_Ogh_sU1-EWe48c_eKTxDO2D80OzI1STJXh5aHkRyjx-9Ievo97x7DLusMcR6Ef0h3hy_DuO6_hyVHQiRUNz43hy5ISuaIlYvErgg_shXTi3sThHhMmtHK1y_FHwTGhCtejRGz5yXUdCKtmM8sKP_nCH_zjI8MGXpUG-QXQkhB5x4z76C82DR_iJvEaL5oc-4WDK4U2gFy-o6E7wo-WC0NuO9EF_9IPIo9fx5IPmw3_w-ER10CQqphiz4jC5HNqP4_GRHxf6Cg59vBaOHz_ShytEDjn6sLhe9IMP9NkRSobIRS9Kb0N3wc6Lv8jzBs_4wdPwnngfXPmIvriLOD-S8OHxPXh0lDp-Cr_hz-iX4z_yC_pjvE2P7-jEC-I1vEY94kPr44fP4LxxfEHKHt8RiSgtfIf54D2O6seDo-EPjVqLvQX6fGim6cMVyag_UF8iqDFBxcfhHpWUCWOOPkVzQdeJJzn-47pQPVsgasdrHLlZmIeP6_CJF0e-w2sh_Dh-6DkeHjfE10a-FN-L6_AvnDve-jiTy-h5RDzkE9meFteLJ6nRczz4D_xBnTlqZcSP_Bb-Q9-FLuEhXkPJ48OPXoG7os9x4Uc_mGpyMAfqQyR3_IF_-Kg4fBcAQIVyQggjgFRCAUOBMgYAAIBTAgmiAAFIAIoYAJQRIgCwigCAJCGGGCMIEExpBxDAAAlBEBOCHOkoJUoB4gAhBTlDiFHYESAEBQAZADAySGvEAHbEOEGIMw6k6qxwgCDBJDMCAAQA44wCoYQAxAECHHAIAOaRSUIYhwCAlBpkgKbOACGIkMAQIhQzREhiAEIEEQAQMgYAAYRViiGEDHKGUcAEIMCJEJQQAggAgCQKAICsA84AgRggUhAnYDEGEWAMER4ogxgSRAAQkQQACAcAYIQARpAWwghCGAIEEAKoBAAIJhQhjAALDCEEQUCUQQwBJwwghDiCABGBGEikUIgQJIQxAAiDABAIOISIItAo5CQiwAjRiLECEMSUEUgBDQBTTAAEgIGWESoQ4hAIQwBxgjkDAFLCDOqUE4gakIQySAAiKKFIDII0IQYZxhSTxhDCjBAMEGsEIcIxBYEBTnAhSHGCGKOAUAgoSBQRylEgkAMgEEMQIpAJBgAwQggFkCAACUQEMkwihYADHABmBRPOIEEAGAIAahgDggBAGAKQYSYAEYYghIxkgihABDTSCEIO5EQhAJAxiADDgARGEmaYEMRgwIwwlDJEmUQAIgMEEIIBJIAgAABBmKJEGACAUgdRoBgACBgjgEVACShEwAwRh4QRAAlkjFQAAIMQIEIQAIRBQAECBCIEKISIMcAIARBQhhCkGEAECqSFApYoQgACghKlhINEAAOY0JQ5aACFgAKBCTAACCaEUMAhRghwhiwjqRGEIIcgUsAgBBRRDCiFCCQCOQMEQIgSJJAixnhomGNGaIAGMNYQARhzBBiDBAAWAUGBEwAhQhQDDgnimBDGQAAAcYQw5xwCDCAnEHEEEIOIcMQASIQwQEkHhEJAKWIQIMQIRgAjTggmCBKSGKaQMEAIg4igABABgBBGCEAMEcAQg4BjAiGAAFCIIGQgUQoqRRCBgghBCEEA";
	const char *fp_roar2 = "AQADtEqiROGiSBJaLhFYksJZIfQI70R7_PhxcUfaKAoeNNzRcjnyb5CSMJKEcCWOZsyN_qgfIhUFsUSlEvlehLzgqvgO0cmWoZR-nDmg6sFj_FmRp8bFLWi-I5SPR8H1fDht6BNahUauScdhOT565J2DJ6KhD82lB-FvnLnx7GCJxh38HA9_nMlh_vjhcUJO6OQRVhdKBhdhlcH8C-0RsiGH-9CNWNNx7PNxUbBDhHKkQvdxLTtCyRaa40f5PGj4Y09GodfhH6F4HLIYkgh_wlXmwdke3A-eYluHB32kFHeEPGRQjWEhjgfSv-jzBOcxhUrG4cJ_hD--ELJ4CqHFHH9SfKqPvcE7-Ogl4pfwpUeoiBG8B4-hX4Xd48-RUpokohQXHFeQhceZQ094xM6FH5Ru4in-9ejR6kdaFT_EG-WDOqFlfMvxE80N9fiRZ8HR5zAPnwfyQ0dKKsAjBp_RE7nQFyYjFDfaCz8-EvzxKriOk_gf4SL8QePxKUd_mFH2Db0Q6hlhNch2FiYeY8yKo-FzHIeeVMdrPIvQ8ML14fihEz-O4CFx4seRHj-OH4ev49rhH_rC2MjhMbiP18ezH-hLFacR54f6IHr0BU8ulMd_fBf8WXiOWsGPXCbUx3iYHloj6fCR5UGOHqYzokdx8AvwHD56POj0HPnxDT-cB2-QMqegnTnYC-Xhr_hxfniCm4f-wIlPcCfwpSrWZbheiDva4zwu3Dp-oXoWIsWFX8iPH6KKL0h_3MdpHDCP-DhUNTvEHz8sGjqP-LxgDh_uW3hMfEOPi1Ms5NCPyE1FPMktVEd9BX1e-EcfpcJxfCWFj4Z9gnsuZFmTQ8yOF_nx41cI79EEPzbCRieS2EpQPUfzPPjxVvh2PMITZSi5oHkeIfGVZ8jF41LQ8D_OFs15_EHzXZg-JA-D_MWDH4wsNimaqsh_xKZRdlEMLzeeSBue6vgDj0dDzkjOH_yQnsan48bb4wgnHuK7oxYfHOgHkxqOi8FvfEfKPMR7BDveI-WDd_iFH_2D-8gHkdkiRL5Y_Hh1NP_QRjqeEuEDzuHA5fjhHpM6QU109Al8IzSufDgtXES8eEGjmMZ14jx0-Pjxw8cn6Ljw4Pjh4xvS63gYpMEX6LWRJ08L47_wisL341eKnsih8Eec44ocBmXiVDhjnCmO50fzEblwpYL64x-eLBe0q3BFFWUPtNzh48PFWESPPjr8HD80b0d__PA6PHhQscd3aNwNJotqCW_B28WDHv8juEIdhsLPo3JGCU5wvXgTvMWW7BC0Cz9yHbfwCy-a7FKRN8J16MWRHzn84gJ04rmRBvmJ40vxCxchyjwQV7fwHD6uH_dx5IdmJUqRKyl-zDl-1N_RuMG3A8935EWvY98CtbyCiXqCa0PzRMH0qviu4ApjlNKhTB4e4LtQLpqFdvAldNCuwyX8RdDKnfjRUFqgiT5-MI_QXEeeSQ5-DSVvpMt-9ESdKTP0OkIlnsETvgiTK-gPjsWzmmhDBZ5PdMZ55DmuKdCPiQ_S5Efl0PhoIagpQ8ulHM9RHmlF4z8sJ-iLRz8aVhYqLTueSESoZ0QjJG3yFbcW1OIDn9rRE3-OZ-KPZziC5CfGNEmFm3h-XD96V4HrMeiPB8eRK1MqQ28uNL9QOcZxZ8KjPYGWH-WRLzkKT9vxHGePV3jw7iHSLjr-4Md_iGnj4D3S6nCpgPkiJ2CnHNeL7wjZQzxxZyxCZ48SPPvAJZ0NRn_go_rxKB_GcSnCTNB-nMfJDjeJf3iKJrrRNRe-qLjR3NAelKzhf7jww3nwoY009PB-9EnQlRnEHOGP61nQw6d29Hg8H3dW3MWPnEjyBk-CJ9mP69hRpxTuOPCvGBOe47gybTnyJpMgVvhp6MoHmjtqMfDhikYe_OAP0_iFdD0O6oHeGIfX4w_-Cl4lhGLwi-iPZhr-DW-QxYerQvQu4fqQvYIvEf8G0oRDXbiWCV90wsfUIz--5Piha6gdwSrRhcnx49AOV1ELpC_OHC_uw6cRkiH-D6GJdFdRVYS2X0hz9Dz6E_7wUaiOb5eDu8URJD925SGe9PiP53jwXjr6hEmG5riQ69Cz49oSJUJzKgkK7ofESQpeiQj1yPA9VFMSGZ_RNYTD48UL70ie_QjjcMZ14UfzHN-P4wEZ5kI-xngH0UHoEb9xHc0t9PiiI-9xtNDiKQcOplxwNdDxgtOdwPbRckO9Hc2T44L2wfRRK4eeD33gOCe6gzuOiinGrPhz-IN2vMNz5MeloflxPniNh_iLH-l5JORx9CyuM-jho3h5hBoPcVQ0BT162HnxXkP-wss_XMfv4A2ufESPXC7U_Ij4VviOJznK4zFV_IIf9OPxI7-gxzHeesSXHJ2mQ_xQKwfOHT1--Awu3viQD2aOH5FooYTJHX-MD6iu4ziaJeeg6BPxHEfzHCGlE3QyHeE8MA4gYAYwTBolHEHECSMMAwIYg4iwTAADsADAMEEkEAQJQxACAgGhALGCAWEQAAoJCQwCzBABRFNCAEEEAZZJIRAFSgEADBIKEIOAkEhAIBwBwhBgACAAKoMIEYBJZwwhhCGiABACEWcAQ4IJyxABpEAghHGSOu4YEg4RBwCBhghACUYCICAdQAAQkQxAGiBkglTCOuuMA51QZ4UDAjiAhGoMKGCMYEAJIYgRDCAHBHVgSCMAwEIUZYEBWjkAhBCSGEKIcJhIwoBBBAkEACDCIaUYQIggZ5SxwigCjAMTCAEMwEABRAACliHnCAAGAYEUkbAYR4BhRBGLFBFAECMAJIhQw4AAwAAAgCCAOSSEIU4Yh4AAUAECjAKGGGEBCYQgAoAjFDHkBCDEEQQIYoAQQwwYEBEgjECUOwQUQooQj5Az0BBhhGUUGSuMMAAJogRACgChBHDCEQSIZqYCIAxHgDhBBCLGIUGEQgYIRQykDjghHKCWAGGEcAIBhhBDjCGkAUAIGcVAcEIQR4ixAgBiiTBAUOCUUAQ0J4xAhBghDBNACSAQMYYiIoASBDlGHDHEIIUMEEQIIwkSBEgkKIFIGAMAMxQAJgQQghnCkCAKQAAAoIgRQAwgiABLGHODAGAIQsAQR4RQiAijBAEGIGcAIQQIg4xBBCBABEBYCKEMA0owALUwgCIBjCMWgAYEwMIqowQhQQBLgdEMAACYSsAw4AowgBghrFQMCSGROAYRKoxAQAHjlEMEMACkAQgwZyQBwlFjABCAGIsIMAAIQsQQAIAkhQCUKAMYIsIQBQABCjQGCSQIUCUNUcw5gQBRDAGCgAGkEGiAE4ghghgIiChFgEFIFWIIlIADYwARGClojEYCQMaE4UAIywgw3BDhrGSEGCEQAkICBJgTTCBFiGOOIIKYNAYCggQyhiGHAALOKUcIEQgISAihBhRhCA";

	ChromaprintMatcherContext *ctx;

	ctx = chromaprint_matcher_new();
	ASSERT_NE(nullptr, ctx);
	SCOPE_EXIT(chromaprint_matcher_free(ctx));

	ASSERT_EQ(1, chromaprint_matcher_set_fingerprint(ctx, 0, fp_roar1));
	ASSERT_EQ(1, chromaprint_matcher_set_fingerprint(ctx, 1, fp_roar2));
	ASSERT_EQ(1, chromaprint_matcher_run(ctx));

	int num_segments;
	ASSERT_EQ(1, chromaprint_matcher_get_num_segments(ctx, &num_segments));
	ASSERT_EQ(1, num_segments);

	int pos1, pos2, duration;
	ASSERT_EQ(1, chromaprint_matcher_get_segment_position_ms(ctx, 0, &pos1, &pos2, &duration));
	ASSERT_EQ(0, pos1);
	ASSERT_EQ(9657, pos2);
	ASSERT_EQ(107714, duration);

	int score;
	ASSERT_EQ(1, chromaprint_matcher_get_segment_score(ctx, 0, &score));
	ASSERT_EQ(7, score);
}

}; // namespace chromaprint
