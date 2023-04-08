#include "silk_engine/entry_point.h"
#include "src/core/my_app.h"
#include "silk_engine/gfx/render_context.h"
#include <gtest/gtest.h>

Application* createApplication(int argc, char** argv)
{
	return new MyApp({ argc, argv });
}

TEST(AppTest, RuntimeTest)
{
	auto app = new MyApp({ 0, nullptr });
	EXPECT_TRUE(RenderContext::getInstance());
	EXPECT_TRUE(RenderContext::getLogicalDevice());
	EXPECT_TRUE(RenderContext::getAllocator());
	delete app;
}

int main(int argc, char** argv)
{
	::testing::InitGoogleTest(&argc, argv);
	int result = RUN_ALL_TESTS();
	return result;
}