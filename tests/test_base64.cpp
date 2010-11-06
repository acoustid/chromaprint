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

TEST(Base64, Base64Decode)
{
	ASSERT_EQ("x", Base64Decode("eA"));
	ASSERT_EQ("xx", Base64Decode("eHg"));
	ASSERT_EQ("xxx", Base64Decode("eHh4"));
	ASSERT_EQ("xxxx", Base64Decode("eHh4eA"));
	ASSERT_EQ("xxxxx", Base64Decode("eHh4eHg"));
	ASSERT_EQ("xxxxxx", Base64Decode("eHh4eHh4"));
	ASSERT_EQ("\xff\xee", Base64Decode("_-4"));
}
