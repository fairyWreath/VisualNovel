#include "Engine.hpp"
#include "Utility.hpp"
#include "Logger.hpp"

#include <SFML/Graphics/RenderWindow.hpp>

#include <algorithm>
#include <iostream>

Engine::Engine(State::Context context) :
	nMusicPlayer(*context.musicPlayer),
	nTextures(*context.textures),
	nFont(context.fonts->get("overlock")),
	nSoundPlayer(*context.soundPlayer),
	nLinePrinted(true),
	nTextPos(-1),
	nFadeTime(0),
	nInFade(false),
	nAutoMode(true),
	nDialogueSpeed(5),
	nTextTime(sf::Time::Zero),
	nDelayTime(sf::Time::Zero),
	nTextInterval(BaseInterval),
	nDelaySpeed(1),
	nDelay(BaseDelay),
	nDialogueBoxOpacity(50),
	nWait(false),
	nHideDialogueBox(false),
	nEntities(),
	nCharacters(),
	nDialogueBox("DialogueBox", context.textures->get("dialoguebox")),
	nBackground("Background", context.textures->get("MMBG"))
{
	nDelay = BaseDelay / nDelaySpeed;
	nTextInterval = BaseInterval / nDialogueSpeed;

	// set up dialogue textures and fonts and text
	nText.setCharacterSize(35);
	nText.setPosition(260.f, 520.f);
	nText.setFillColor(sf::Color::Black);
	nText.setFont(context.fonts->get("overlock"));

	nCharName.setCharacterSize(35);
	nCharName.setPosition(130.f, 520.f);
	nCharName.setFillColor(sf::Color::Black);
	nCharName.setFont(context.fonts->get("overlock"));

	sf::Texture& dbT = nTextures.get("dialoguebox");
	float dbx = (float)context.window->getSize().x / 2 - (float)dbT.getSize().x / 2;
	float dby = (float)context.window->getSize().y - (float)dbT.getSize().y - 30.f;
	nDialogueBox.setPosition(dbx, dby);

	setDialogueBoxOpacity(nDialogueBoxOpacity);

	nEntities.reserve(10);			// RESERVE WITH THE SIZE OF THE SPRITES???

	nBackground.setPosition(0.f, 0.f);
	// nBackground.fade(3.f, 255, 1);
	// nDialogueBox.fade(3.f, 255, 1);

	//auto myr1 = std::make_unique<Entity>("Mayuri1", nTextures.get("mayuri1"));
	//auto myr2 = std::make_unique<Entity>("Mayuri2", nTextures.get("mayuri2"));
	//myr1->setPosition(700.f, 40.f);
	//myr2->setPosition(700.f, 40.f);
	//addEntity(std::move(myr1));
	
	
	// myr2->blur(2.f, 0.f, 100.f);
	// addEntity(std::move(myr2));
	//fadeEntity("Mayuri1", 1.f, 0, 255);
	//fadeEntity("Mayuri2", 1.f, 255, 1);

	// setWaitAnimation(true);

	// moveEntity("Mayuri1", 1.f, sf::Vector2f(700.f, 40.f), sf::Vector2f(900.f, 40.f));
	// moveEntity("Mayuri2", 1.f, sf::Vector2f(700.f, 40.f), sf::Vector2f(900.f, 40.f));

	//auto ctr = std::make_unique<Character>("Mayuri", nTextures.get("mayuri1"), "default");
	// ctr->setPosition(sf::Vector2f(700, 40));
	// Character* ptr = ctr.get();
	// nEntities.push_back(std::move(ctr));
	// ptr->setOpacityAlpha(255);
	// ptr->fade(2, 255, 0);
	// ptr->insertState("second", nTextures.get("mayuri2"));
	// ptr->setState("second", 0.3);
	// ptr->setState("default");
	// ptr->move(0.75, sf::Vector2f(700, 40));

	// fadeScreen(3.f, 0);

	/* character test  */
	CharacterBlueprint bp("mayuri", "state1", "mayuri1", nTextures);
	bp.insertState("state2", "mayuri2");
	CharacterPtr mayuri1 = std::make_unique<Character>(bp);

	nCharacters.push_back(std::move(mayuri1));

	testClock.restart();
}

void Engine::draw(sf::RenderTarget& target, sf::RenderStates states)  const
{
	target.draw(nBackground, states);

	for (const auto& ch : nCharacters) target.draw(*ch, states);
	//for (const auto& ent : nEntities) target.draw(*ent, states);
	
	target.draw(nDialogueBox, states);
	target.draw(nCharName, states);
	target.draw(nText, states);
}

void Engine::update(sf::Time dt)
{
	if (testClock.getElapsedTime().asSeconds() > 3 && test1)
	{
		Character* mayuri = getCharacter("mayuri");
		mayuri->setState("state2", 2.0f);
		test1 = false;
	}
	
	if (testClock.getElapsedTime().asSeconds() > 8 && test2)
	{
		Character* mayuri = getCharacter("mayuri");
		mayuri->setState("state1", 2.0f);
		test2 = false;
	}

	if (!nLinePrinted) nTextTime += dt;

	if (!nLinePrinted && nTextTime.asMilliseconds() > nTextInterval) {
		nTextTime += dt;
		nTextPos++;
		if (nTextPos >= nTextString.size()) {
			nLinePrinted = true;
			nDelayTime = sf::Time::Zero;
			nTextTime = sf::Time::Zero;
		}
		else {
			std::string str = nText.getString();
			str += nTextString[nTextPos];
			size_t pos = nTextPos;
			while (nTextString[pos] != ' ' && pos < nTextString.size()) pos++;

			sf::String wrapped = wrapText(sf::String(str), 800, nFont, 35, (int)(pos - nTextPos));
			nText.setString(wrapped);
		}
		nTextTime = sf::Time::Zero;
	}
	else if (nAutoMode && nLinePrinted) {
		if (nDelayTime.asMilliseconds() > nDelay) {
			nWait = false;
		}
		else {
			nDelayTime += dt;
		}
	}
	else {
		// empty
	}

	nBackground.update(dt);
	nDialogueBox.update(dt);

	for (const auto& ent : nCharacters) ent->update(dt);
	for (const auto& ent : nEntities) ent->update(dt);

	// clearTransparentEntities();

	if (nAnimationWait)
	{
		if (isInAnimation()) nWait = true;
		else
		{
			nWait = false;
			nAnimationWait = false;
		}
	}
}

bool Engine::handleEvent(const sf::Event& event)
{
	if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Enter)
	{
		// case to case basis???
		if (!nLinePrinted)
		{
			skipDialogueLine();
		}
	//	if (isInAnimation()) skipAnimations();

		nWait = false;
	}
	else if (event.type == sf::Event::MouseButtonReleased && event.key.code == sf::Mouse::Left)
	{
		// case to case basis???
		if (!nLinePrinted)
		{
			skipDialogueLine();
		}
	//	if (isInAnimation()) skipAnimations();

		nWait = false;
	}
	return false;
}

void Engine::playMusic(const std::string& id)
{
	nMusicPlayer.play(id);
}

void Engine::playSound(const std::string& id)
{
	nSoundPlayer.play(id);
}

void Engine::stopMusic()
{
	nMusicPlayer.stop();
}

void Engine::setDialogueSpeed(int amount)
{
	if (amount < 1 && amount > 10) return;
	nDialogueSpeed = 5;
	nTextInterval = BaseInterval / nDialogueSpeed;
}

void Engine::setAuto(bool autoState)
{
	nAutoMode = autoState;
}

void Engine::skipDialogueLine()
{
	sf::String wrapped = wrapText(sf::String(nTextString), 800, nFont, 35, 0);
	nText.setString(wrapped);
	nLinePrinted = true;
	nDelayTime = sf::Time::Zero;
}

void Engine::skipAnimations()
{
	if (nBackground.inAnimation()) nBackground.skipAnimation();
	if (nDialogueBox.inAnimation()) nDialogueBox.skipAnimation();

	for (const auto& ent : nEntities)
	{
		if (ent->inAnimation()) ent->skipAnimation();
	}

	setWaitAnimation(false);
}

bool Engine::isInAnimation() const
{
	if (nBackground.inAnimation()) return true;
	if (nDialogueBox.inAnimation()) return true;

	for (const auto& ent : nEntities)
	{
		if (ent->inAnimation()) return true;
	}
	
	return false;
}


bool Engine::shouldWait() const
{
	return nWait;
}

void Engine::setBackground(const std::string& id)
{
	nBackground.setTexture(nTextures.get(id));
}

void Engine::displayText(const std::string& text, const std::string& name)
{
	nTextString = text;
	nTextPos = -1;
	nLinePrinted = false;
	nWait = true;
	nText.setString("");
	nCharName.setString(name);
}

void Engine::addEntity(const std::string& id, const std::string& texture, const sf::Vector2f& pos)
{
	// for now assume all to be added entities are new
	EntityPtr ent = std::make_unique<Entity>(id, nTextures.get(texture));
	ent->setPosition(pos);
	nEntities.push_back(std::move(ent));
}

void Engine::removeEntity(const std::string& id)
{
	std::cout << "sprite to remove: " << id << std::endl;
	auto test = nEntities.cbegin();

	for (auto itr = nEntities.cbegin(); itr != nEntities.cend();)
	{
		if ((*itr)->getIdentifier() == id)
		{
			std::cout << "sprite found\n";
			itr = nEntities.erase(itr);
			//	return;		// remove all instances of the same identifier
		}
		else itr++;
	}
}


void Engine::fadeScreen(float time, int targetAlpha, int startAlpha)
{
	nBackground.fade(time, targetAlpha, startAlpha);
	nDialogueBox.fade(time, targetAlpha, startAlpha);

	for (const auto& ent : nEntities) ent->fade(time, targetAlpha, startAlpha);
}

void Engine::clearTransparentEntities()
{
	for (auto itr = nEntities.begin(); itr != nEntities.end(); )
	{
		if ((*itr)->getOpacityAlpha() <= 0)
		{
			itr = nEntities.erase(itr);
			std::cout << "aplha 0 entity removed\n";
		}
		else
			itr++;
	}

//	std::cout << "size after entity clerance: " << nEntities.size() << std::endl;
}

Character* Engine::getCharacter(const std::string& id)
{
	const auto ch = std::find_if(nCharacters.begin(), nCharacters.end(), [&id](const CharacterPtr& ptr)
		{
			return (ptr->getIdentifier() == id);
		});

	if (ch != nCharacters.end()) return (*ch).get();
	
	// add log errorhere
	return nullptr;
}

Entity* Engine::getEntity(const std::string& id)
{
	for (const auto& ent : nEntities)
	{
		if (ent->getIdentifier() == id)
			return ent.get();
	}

	return nullptr;
}

void Engine::addEntity(EntityPtr entity)
{
	nEntities.push_back(std::move(entity));
}


void Engine::setDialogueBoxOpacity(float amount)
{
	// set opacity, 100% = 225
	nDialogueBoxOpacity = amount;
	float alpha = (amount / (float)100) * (float)255;
	nDialogueBox.setOpacityAlpha((int)alpha);
}

void Engine::setWait(bool w)
{
	nWait = w;
}

void Engine::setWaitAnimation(bool w)
{
	nAnimationWait = w;
	if (w) nWait = true;		// set wait true the first time around just in case
}
