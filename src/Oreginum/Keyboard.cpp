#include "Window.hpp"
#include "Keyboard.hpp"

namespace
{
	std::string buffer;
	bool pressed[255];
}

void Oreginum::Keyboard::update()
{
	buffer.clear();
	for(bool& k : pressed) k = false;
}

void Oreginum::Keyboard::set_pressed(Key key, bool pressed)
{
	::pressed[key] = pressed;

	char character{to_character(key)};
	if(character) buffer += character;
}

char Oreginum::Keyboard::to_character(Key key)
{
    bool shift{Oreginum::Keyboard::is_held(Oreginum::Key::SHIFT)};

	switch(key)
	{
	case SPACEBAR: return ' '; break;
    case ZERO: return shift ? ')' : '0'; break;
    case ONE: return shift ? '!' : '1'; break;
    case TWO: return shift ? '@' : '2'; break;
    case THREE: return shift ? '#' : '3'; break;
    case FOUR: return shift ? '$' : '4'; break;
    case FIVE: return shift ? '%' : '5'; break;
    case SIX: return shift ? '^' : '6'; break;
    case SEVEN: return shift ? '&' : '7'; break;
    case EIGHT: return shift ? '*' : '8'; break;
    case NINE: return shift ? '(' : '9'; break;
    case A: return shift ? 'A' : 'a'; break;
    case B: return shift ? 'B' : 'b'; break;
    case C: return shift ? 'C' : 'c'; break;
    case D: return shift ? 'D' : 'd'; break;
    case E: return shift ? 'E' : 'e'; break;
    case F: return shift ? 'F' : 'f'; break;
    case G: return shift ? 'G' : 'g'; break;
    case H: return shift ? 'H' : 'h'; break;
    case I: return shift ? 'I' : 'i'; break;
    case J: return shift ? 'J' : 'j'; break;
    case K: return shift ? 'K' : 'k'; break;
    case L: return shift ? 'L' : 'l'; break;
    case M: return shift ? 'M' : 'm'; break;
    case N: return shift ? 'N' : 'n'; break;
    case O: return shift ? 'O' : 'o'; break;
    case P: return shift ? 'P' : 'p'; break;
    case Q: return shift ? 'Q' : 'q'; break;
    case R: return shift ? 'R' : 'r'; break;
    case S: return shift ? 'S' : 's'; break;
    case T: return shift ? 'T' : 't'; break;
    case U: return shift ? 'U' : 'u'; break;
    case V: return shift ? 'V' : 'v'; break;
    case W: return shift ? 'W' : 'w'; break;
    case X: return shift ? 'X' : 'x'; break;
    case Y: return shift ? 'Y' : 'y'; break;
    case Z: return shift ? 'Z' : 'z'; break;
    case SEMICOLON: return shift ? ': ' : ';'; break;
    case PLUS: return shift ? '+' : '='; break;
    case COMMA: return shift ? '<,' : ','; break;
    case MINUS: return shift ? '_' : '-'; break;
    case PERIOD: return shift ? '>' : '.'; break;
    case SLASH: return shift ? '?' : '/'; break;
    case TILDE: return shift ? '`' : '~'; break;
    case LEFT_BRACKET: return shift ? '{' : '['; break;
    case BACKSLASH: return shift ?  '|' : '\\'; break;
    case RIGHT_BRACKET: return shift ? '}' : ']'; break;
    case QUOTE: return shift ? '"' : '\''; break;
	default: return NULL;
	}
}

bool Oreginum::Keyboard::was_pressed(Key key){ return pressed[key]; }

bool Oreginum::Keyboard::is_held(Key key)
{ return (GetAsyncKeyState(key) != 0) && Window::has_focus(); }

std::string& Oreginum::Keyboard::get_buffer(){ return buffer; }