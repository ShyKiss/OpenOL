#pragma once

// Called from game-thread ticker — copies UConsole::Scrollback into snap buffer.
void OLImGui_Console_Tick();

// Called from render thread inside the main ImGui::Begin/End — draws console as a child widget.
void OLImGui_Console_Draw();
