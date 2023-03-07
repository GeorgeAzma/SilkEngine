#include <gtest/gtest.h>
#include "src/core/my_app.h"
#include "silk_engine/gfx/graphics.h"

TEST(AppTest, RuntimeTest)
{
	auto app = new MyApp({ 0, nullptr });
	EXPECT_TRUE(Graphics::instance);
	EXPECT_TRUE(Graphics::logical_device);
	EXPECT_TRUE(Graphics::allocator);
	EXPECT_TRUE(Graphics::command_queue);
	delete app;
}

int main(int argc, char** argv)
{
	::testing::InitGoogleTest(&argc, argv);
	int result = RUN_ALL_TESTS();
	return result;
}