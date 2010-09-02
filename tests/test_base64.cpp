#include <gtest/gtest.h>
#include "base64.h"

using namespace std;
using namespace Chromaprint;

TEST(Base64, Base64Encode)
{
	ASSERT_EQ("eA", Base64Encode("x"));
	ASSERT_EQ("eHg", Base64Encode("xx"));
	ASSERT_EQ("eHh4", Base64Encode("xxx"));
	ASSERT_EQ("eHh4eA", Base64Encode("xxxx"));
	ASSERT_EQ("eHh4eHg", Base64Encode("xxxxx"));
	ASSERT_EQ("eHh4eHh4", Base64Encode("xxxxxx"));
	ASSERT_EQ("_-4", Base64Encode("\xff\xee"));
}
