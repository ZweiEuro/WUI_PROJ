#include <spdlog/spdlog.h>
#include <stdio.h>
#include <allegro5/allegro.h>
#include <allegro5/allegro_x.h>

#ifdef ALLEGRO_WINDOWS
#include <allegro5/allegro_windows.h>
#endif

#ifdef ALLEGRO_UNIX
#include <allegro5/allegro_x.h>
#endif

#include "Objects/Ball.hpp"
#include "Input/Input.hpp"
#include "Renderer/Renderer.hpp"

#include "webUi.hpp"
#include "webUiBinding.hpp"

int main(int argc, char *argv[])
{
	// first thing to call in your program (Internal Fork)
	wui::WuiInit(argc, argv);

	// init renderer and display

	if (!al_init())
	{
		spdlog::error("Failed to initialize allegro");
		return 1;
	}

	input::start();
	render::renderer.start();

	// esc shutdown
	std::thread([=]() -> void
				{
					auto ok = input::wait_for_key(ALLEGRO_KEY_ESCAPE);

					if (!ok)
					{
						spdlog::info("Stop esc listener");
						return;
					}

					spdlog::info("[Main] Shutting down");
					wui::shutdown();
					render::renderer.shutdown();
					input::shutdown();
					
					render::renderer.waitUntilEnd();
					exit(0); // clean exit
					return; })
		.detach();

	// click listener
	std::thread([=]() -> void
				{
                	while (true)
                	{
						vec2i pos;
                    	auto ok = input::wait_for_mouse_button(2, pos);
						if (!ok)
						{
							spdlog::info("Stop click listener");
							return;
						}

						spdlog::info("Adding ball at {} {}", pos.x, pos.y);
						render::renderer.addObject(std::make_shared<objects::Ball>(pos.x, pos.y));
				  	}
				return; })
		.detach();

	// debug 'o' key
	std::thread([=]() -> void
				{
		while (true)
		{
			auto ok = input::wait_for_key(ALLEGRO_KEY_O);

			if (!ok)
			{
				spdlog::info("Stop O listener");
				return;
			}

			if (render::renderer.wui_tab_id>0){
				auto tmp = render::renderer.wui_tab_id;
			render::renderer.wui_tab_id = 0;
			WUI_ERROR_CHECK(wui::closeOffscreenTab(tmp));
			spdlog::info("Destroyed tab {}", tmp);
			}

		
					}
				return; })
		.detach();

	// r key restart UI
	std::thread([=]() -> void
				{
		while (true)
		{
			auto ok = input::wait_for_key(ALLEGRO_KEY_R);

			if (!ok)
			{
				spdlog::info("Stop R listener");
				return;
			}

					render::renderer.restartWui();
		
					}
				return; })
		.detach();

	wui::runTimeLoop();

	pthread_exit(NULL);
}