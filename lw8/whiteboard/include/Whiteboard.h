#ifndef WHITEBOARD_WHITEBOARD_H
#define WHITEBOARD_WHITEBOARD_H

#include <SFML/Graphics.hpp>
#include "Server.h"
#include "Client.h"
#include <memory>

class Whiteboard
{
public:
	Whiteboard();
	void RunServer(short port);
	void RunClient(const std::string& host, short port);

private:
	void MainLoop();
	void ProcessInput();
	void Update();
	void Render();
	void AddLineToCanvas(const DrawData& data);

	sf::RenderWindow m_window;
	sf::RenderTexture m_canvas;
	sf::Sprite m_canvasSprite;

	sf::Color m_currentColor = sf::Color::Black;
	sf::Vector2f m_lastMousePos;
	bool m_isDrawing = false;

	std::unique_ptr<Server> m_server;
	std::unique_ptr<Client> m_client;
};

#endif // WHITEBOARD_WHITEBOARD_H
