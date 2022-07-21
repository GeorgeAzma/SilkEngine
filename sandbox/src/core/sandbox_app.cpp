#include "sandbox_app.h"

SandboxApp::SandboxApp(ApplicationCommandLineArgs args)
{  
    scene = makeShared<Scene>();   
    SceneManager::add(scene);
    SceneManager::switchTo(scene);

    Resources::addImage("Test1", makeShared<Image2D>("test1.png"));
    Resources::addImage("Test2", makeShared<Image2D>("test2.png"));
    Resources::addModel("Backpack", makeShared<Model>("backpack/backpack.obj"));
    
    Timers::every(100ms, 
        [this] 
        { 
            Window::setTitle(fmt::format("Vulkan - {0} FPS ({1:.4} ms) | {2}x{3}", int(1.0 / Time::dt), (Time::dt * 1000), Window::getWidth(), Window::getHeight()));
        });   

    Window::setSize({ 800, 600 });
    camera = scene->createEntity();
    camera->add<CameraComponent>();
    camera->add<ScriptComponent>().bind<CameraController>();

    audio_device = makeShared<AudioDevice>();

    AudioManager::setDistanceModel(DistanceModel::NONE);
    audio = makeShared<Audio>("bounce.wav");
    audio_source = makeShared<AudioSource>();
    microphone = makeShared<Microphone>();
    microphone->start();
}

void SandboxApp::onUpdate()
{  
    if (Input::isKeyPressed(Keys::X) && entities.size())
    {
        entities.erase(entities.begin());
    }
    if (Input::isKeyPressed(Keys::Z)) 
    {
        entities.emplace_back(scene->createEntity());
        entities.back()->add<MaterialComponent>(Resources::getGraphicsPipeline("Lit 3D"));
        entities.back()->add<ColorComponent>(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
        entities.back()->add<TransformComponent>(glm::translate(glm::mat4(1.0f), glm::vec3(RNG::Float(), RNG::Float(), RNG::Float()) * 20.0f));
        entities.back()->add<ModelComponent>(Resources::getModel("Backpack"));
        Light light{};
        light.color = glm::vec3(1);
        entities.back()->add<LightComponent>(light);
    }
    if (Input::isKeyPressed(Keys::F2))
    {
        Graphics::screenshot(fmt::format("data/images/screenshots/screenshot.png"));
    }

    uint32_t samples = microphone->getAvailableSampleCount();
    if (samples > 0)
    {
        DebugTimer t;
        if (audio_source->getQueuedBufferCount() < 3)
        {
            std::vector<uint8_t> buffer(samples * 2);
            microphone->getSamples(buffer.data(), samples);

            RawAudio a{};
            a.format = AudioFormat::MONO16;
            a.sample_rate = 44100;
            a.data = buffer;
            audio = makeShared<Audio>(a);
            audio_source->queue(*audio);
        }
        if (!audio_source->isPlaying())
            audio_source->play();
        uint32_t id = audio_source->unqueue();
    }

    SK_TRACE("Q: {} | P: {}", audio_source->getQueuedBufferCount(), audio_source->getProcessedBufferCount());

    //const auto& cam = camera->get<CameraComponent>().camera;
    //AudioManager::getListener().setPosition(cam.position);
    //
    ////0,0,0 = front | 90, 0, 0 = left
    //AudioManager::getListener().setRotation({ cam.rotation.x + glm::pi<float>(), cam.rotation.y, 0.0f });
    //
    //Renderer::text(fmt::format("Pos {}, {}", cam.position.x, cam.position.y), 30.0f, 60.0f, 20.0f);
    //Renderer::text(fmt::format("Rot {}, {}", cam.rotation.x + glm::pi<float>(), cam.rotation.y), 30.0f, 30.0f, 20.0f);
    //
    //float x = 0.0f;
    //Renderer::sphere(x, 0.0f, 0.0f, 0.2f);
    //audio_source->setPosition({ x, 0, 0});
}

SandboxApp::~SandboxApp()
{
    microphone->stop();
    SceneManager::remove(scene);
}

//CREATE APP
Application* createApp(ApplicationCommandLineArgs args)
{
    return new SandboxApp(args);
}
