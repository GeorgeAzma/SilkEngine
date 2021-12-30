extern Application *createApp();

int main(int argc, char **argv)
{
    auto app = createApp();
    app->run();
    delete app;
}