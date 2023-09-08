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

int main(int argc, char *argv[])
{

	// init renderer and display

	render::Renderer::instance().start();

	// create browser-window

	input::init();

	// esc shutdown
	std::thread([=]() -> void
				{
                  input::wait_for_key(ALLEGRO_KEY_ESCAPE);
				  // spdlog::info("Shutting down");
				  render::Renderer::instance().shutdown();
					  exit(0); })
		.detach();

	// click listener

	std::thread([=]() -> void
				{
                  while (true)
                  {
                    vec2i pos;
                    auto ok = input::wait_for_mouse_button(2, pos);
                    if (!ok)
                      return;

					// spdlog::info("Adding ball at {} {}", pos.x, pos.y);

					render::Renderer::instance().addObject(std::make_shared<objects::Ball>(pos.x, pos.y));


				  } })
		.detach();

	render::Renderer::instance().waitUntilEnd();

	return 0;
}