#include <gtest/gtest.h>
#include "src/core/my_app.h"
#include "silk_engine/gfx/graphics.h"

TEST(AppTest, RuntimeTest)
{
	auto app = new MyApp({ 0, nullptr });
	EXPECT_TRUE(RenderContext::instance);
	EXPECT_TRUE(RenderContext::logical_device);
	EXPECT_TRUE(RenderContext::allocator);
	EXPECT_TRUE(RenderContext::command_queue);
	delete app;
}

int main(int argc, char** argv)
{
	::testing::InitGoogleTest(&argc, argv);
	int result = RUN_ALL_TESTS();
	return result;
}