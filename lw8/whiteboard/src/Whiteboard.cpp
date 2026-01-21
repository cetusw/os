#include "Whiteboard.h"
#include "Color.h"

Whiteboard::Whiteboard()
{
	m_window.create(sf::VideoMode(800, 600), "Whiteboard");
	m_window.setFramerateLimit(60);

	m_canvas.create(800, 600);
	m_canvas.clear(sf::Color::White);
	m_canvas.display();
	m_canvasSprite.setTexture(m_canvas.getTexture());
}

void Whiteboard::RunServer(short port)
{
	m_server = std::make_unique<Server>(port);
	m_server->Start();
	MainLoop();
}

void Whiteboard::RunClient(const std::string& host, short port)
{
	m_client = std::make_unique<Client>(host, port);
	m_client->Start();
	MainLoop();
}

void Whiteboard::MainLoop()
{
	while (m_window.isOpen())
	{
		ProcessInput();
		Update();
		Render();
	}
}

void Whiteboard::ProcessInput()
{
	sf::Event event;
	while (m_window.pollEvent(event))
	{
		if (event.type == sf::Event::Closed)
		{
			m_window.close();
		}

		if (event.type == sf::Event::KeyPressed)
		{
			if (event.key.code == sf::Keyboard::R)
			{
				m_currentColor = sf::Color::Red;
			}
			if (event.key.code == sf::Keyboard::G)
			{
				m_currentColor = sf::Color::Green;
			}
			if (event.key.code == sf::Keyboard::B)
			{
				m_currentColor = sf::Color::Blue;
			}
			if (event.key.code == sf::Keyboard::D)
			{
				m_currentColor = sf::Color::Black;
			}
		}

		if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left)
		{
			m_isDrawing = true;
			m_lastMousePos = m_window.mapPixelToCoords(sf::Mouse::getPosition(m_window));
		}

		if (event.type == sf::Event::MouseButtonReleased)
		{
			m_isDrawing = false;
		}
	}
}

void Whiteboard::Update()
{
	if (m_isDrawing)
	{
		const sf::Vector2f currentPos = m_window.mapPixelToCoords(sf::Mouse::getPosition(m_window));
		if (currentPos != m_lastMousePos)
		{
			const DrawData data(
				m_lastMousePos.x,
				m_lastMousePos.y,
				currentPos.x,
				currentPos.y,
				Color::ToUint(m_currentColor));

			AddLineToCanvas(data);

			if (m_server)
			{
				m_server->SendData(data);
			}
			if (m_client)
			{
				m_client->SendData(data);
			}

			m_lastMousePos = currentPos;
		}
	}

	if (m_client)
	{
		DrawData data;
		while (m_client->PopData(data))
		{
			AddLineToCanvas(data);
		}
	}

	if (m_server)
	{
		DrawData data;
		while (m_server->PopData(data))
		{
			AddLineToCanvas(data);
		}
	}
}

void Whiteboard::AddLineToCanvas(const DrawData& data)
{
	const sf::Vertex line[] = {
		sf::Vertex(sf::Vector2f(data.startX, data.startY), Color::FromUint(data.color)),
		sf::Vertex(sf::Vector2f(data.endX, data.endY), Color::FromUint(data.color))
	};
	m_canvas.draw(line, 2, sf::Lines);
	m_canvas.display();
}

void Whiteboard::Render()
{
	m_window.clear();
	m_window.draw(m_canvasSprite);
	m_window.display();
}