#include <iostream>
#include <string>
#include <vector>
#include <filesystem>
#include <SFML/Audio.hpp>
#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Slider.H>
#include <Windows.h>

void HideConsole()
{
    ::ShowWindow(::GetConsoleWindow(), SW_HIDE);
}
void ShowConsole()
{
    ::ShowWindow(::GetConsoleWindow(), SW_SHOW);
}
bool IsConsoleVisible()
{
    return ::IsWindowVisible(::GetConsoleWindow()) != FALSE;
}

namespace fs = std::filesystem;
using namespace std;
using namespace sf;

class AudioPlayer {
private:
    vector<string> file_names;
    SoundBuffer buffer;
    Sound sound;
    int current_track_index;

    Fl_Box* track_info;

    void updateTrackInfo() {
        if (current_track_index < file_names.size()) {
            string track_name = file_names[current_track_index];
            Time duration = buffer.getDuration();
            int minutes = static_cast<int>(duration.asSeconds()) / 60;
            int seconds = static_cast<int>(duration.asSeconds()) % 60;

            string info = "Playing: " + track_name + " (" + to_string(minutes) + ":" + (seconds < 10 ? "0" : "") + to_string(seconds) + ")";
            track_info->label(info.c_str());
        }
    }

    void loadTrack(int index) {
        if (index >= 0 && index < file_names.size()) {
            current_track_index = index;
            string track_path = "music/" + file_names[current_track_index];

            if (buffer.loadFromFile(track_path)) {
                sound.setBuffer(buffer);
                updateTrackInfo();
            }
            else {
                cerr << "Failed to load track: " << track_path << endl;
            }
        }
    }

    static void playCallback(Fl_Widget*, void* data) {
        static_cast<AudioPlayer*>(data)->sound.play();
    }

    static void pauseCallback(Fl_Widget*, void* data) {
        static_cast<AudioPlayer*>(data)->sound.pause();
    }

    static void nextCallback(Fl_Widget*, void* data) {
        AudioPlayer* player = static_cast<AudioPlayer*>(data);
        int next_index = (player->current_track_index + 1) % player->file_names.size();
        player->loadTrack(next_index);
        player->sound.play();
    }

    static void prevCallback(Fl_Widget*, void* data) {
        AudioPlayer* player = static_cast<AudioPlayer*>(data);
        int prev_index = (player->current_track_index - 1 + player->file_names.size()) % player->file_names.size();
        player->loadTrack(prev_index);
        player->sound.play();
    }

    static void volumeCallback(Fl_Widget* widget, void* data) {
        Fl_Slider* slider = static_cast<Fl_Slider*>(widget);
        AudioPlayer* player = static_cast<AudioPlayer*>(data);
        player->sound.setVolume(static_cast<float>(slider->value()));
    }

public:
    AudioPlayer(const string& folder_path, Fl_Window* window) : current_track_index(0), track_info(nullptr) {
        try {
            for (const auto& entry : fs::directory_iterator(folder_path)) {
                if (entry.is_regular_file() && (entry.path().extension() == ".ogg" || entry.path().extension() == ".wav" || entry.path().extension() == ".mp3")) {
                    file_names.push_back(entry.path().filename().string());
                }
            }

            if (file_names.empty()) {
                throw runtime_error("No audio files found in folder: " + folder_path);
            }

            track_info = new Fl_Box(10, 10, 630, 30, "Loading track...");
            track_info->box(FL_FLAT_BOX);
            track_info->labelsize(14);
            track_info->align(FL_ALIGN_INSIDE | FL_ALIGN_LEFT);

            loadTrack(0);

            Fl_Button* play_button = new Fl_Button(10, 245, 150, 45, "Play  @>");
            play_button->callback(playCallback, this);

            Fl_Button* pause_button = new Fl_Button(170, 245, 150, 45, "Pause  @||");
            pause_button->callback(pauseCallback, this);

            Fl_Button* prev_button = new Fl_Button(330, 245, 150, 45, "Previous  @<<");
            prev_button->callback(prevCallback, this);

            Fl_Button* next_button = new Fl_Button(490, 245, 150, 45, "Next  @>>");
            next_button->callback(nextCallback, this);

            Fl_Slider* volume_slider = new Fl_Slider(610, 50, 20, 180, "Volume");
            volume_slider->type(FL_VERTICAL);
            volume_slider->bounds(0, 100);
            volume_slider->value(100);
            volume_slider->callback(volumeCallback, this);

            sound.setVolume(100); // Default volume
        }
        catch (const exception& e) {
            cerr << e.what() << endl;
            exit(1);
        }
    }
};

int main(int argc, char** argv) {
    HideConsole();
    Fl_Window* window = new Fl_Window(650, 300, "FLTK Audioplayer");

    AudioPlayer player("music", window);

    window->end();
    window->show(argc, argv);

    return Fl::run();
}