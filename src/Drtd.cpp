#include "Drtd.hpp"
#include <Fl/Fl.H>
#include <alsa/asoundlib.h>
#include <cassert>
#include <csignal>
#include <decoder/ax25/Ax25.hpp>
#include <decoder/dcf77/Dcf77.hpp>
#include <decoder/dtmf/Dtmf.hpp>
#include <decoder/null/Null.hpp>
#include <decoder/pocsag/Pocsag.hpp>
#include <decoder/rtty/Rtty.hpp>
#include <memory>
#include <stdio.h>
#include <thread/SoundCardThread.hpp>
#include <thread/StdinThread.hpp>
#include <ui/BiquadFilterDialog.hpp>
#include <ui/ConfigDialog.hpp>
#include <ui/FirFilterDialog.hpp>
#include <ui/IQMixerDialog.hpp>
#include <ui/MainGui.hpp>
#include <ui/MovingAverageDialog.hpp>
#include <ui/WaterfallDialog.hpp>
#include <ui/component/Waterfall.hpp>
#include <util/Config.hpp>
#include <util/DrtdIcon.cpp>
#include <util/Logger.hpp>
#include <util/Util.hpp>
#include <vector>

struct Options {
    static constexpr i8 input_none_specified = -1;
    static constexpr i8 input_show_available = -2;
    bool read_stdin { false };
    bool input_big_endian { false };
    Dsp::StdinThread::SampleSize samples_size { Dsp::StdinThread::SampleSize::S8 };
    SampleRate input_sample_rate { 44100 };
    u8 headless_decoder_index { 0 };
    i8 input_index { input_none_specified };
    Util::Buffer<std::string> decoder_parameters {};
    bool ui_mode { true };
};

constexpr const char* s_conf_audioline = "Drtd.AudioLine";
constexpr const char* s_conf_main_window = "Drtd.MainGui";
constexpr const char* s_conf_decoder_index = "Drtd.LastUsedDecoderIndex";
constexpr const char* s_default_line_name = "default";
const Util::Logger s_log("drtd");

template<typename... Ts>
Buffer<std::shared_ptr<Dsp::DecoderBase>> make_decoder_buffer(Ts&&... ts) {
    return { std::make_shared<Ts>(std::move(ts))... };
}

Buffer<std::shared_ptr<Dsp::DecoderBase>> s_decoders;
u8 s_audio_line_index { 0 };
u8 s_active_decoder_index { 0 };
Ui::MainGui* s_main_gui { nullptr };
Buffer<Drtd::AudioLine> s_audio_lines;
std::unique_ptr<Dsp::ProcessingThread> s_processing_thread;
Options s_options;
Drtd::AudioLine s_default_audio_line;
Fl_RGB_Image s_drtd_icon(drtd_icon_data.pixel_data, drtd_icon_data.width, drtd_icon_data.height, drtd_icon_data.bytes_per_pixel);

Fl_RGB_Image* Drtd::drtd_icon() {
    return &s_drtd_icon;
}

void Drtd::monitor_sample(float sample) {
    Drtd::main_gui().monitor(sample);
}

bool Drtd::using_ui() {
    return s_main_gui;
}

std::shared_ptr<Dsp::DecoderBase> Drtd::active_decoder() {
    return s_decoders[s_active_decoder_index];
}

u8 Drtd::active_decoder_index() {
    return s_active_decoder_index;
}

u8 Drtd::current_audio_line() {
    return s_audio_line_index;
}

const Util::Buffer<Drtd::AudioLine>& Drtd::audio_lines() {
    return s_audio_lines;
}

Ui::MainGui& Drtd::main_gui() {
    assert(s_main_gui);
    return *s_main_gui;
}

void Drtd::for_each_decoder(std::function<void(Dsp::DecoderBase&)> callback) {
    for (auto& decoder : s_decoders)
        callback(*decoder);
}

const Drtd::AudioLine& Drtd::default_audio_line() {
    return s_default_audio_line;
}

bool Drtd::switch_audio_line(u8 audio_line_index) {
    stop_processing();
    s_audio_line_index = audio_line_index;
    return start_processing(s_active_decoder_index);
}

bool Drtd::start_processing(u8 index) {
    assert(index < s_decoders.size());
    assert(!s_processing_thread || !s_processing_thread->is_running());

    s_active_decoder_index = index;
    auto& decoder = s_decoders[s_active_decoder_index];
    s_log.info() << "start_processing(): Decoder \"" << decoder->name() << "\" S/R " << decoder->input_sample_rate();

    if (using_ui())
        main_gui().set_status("Ready.");

    decoder->setup();
    if (using_ui()) {
        auto& gui = main_gui();
        const int min_window_height = std::max(static_cast<int>(Ui::MainGui::default_window_height),
                                               Ui::MainGui::window_padding + decoder->min_ui_height());
        gui.size_range(Ui::MainGui::default_window_width, min_window_height);
        gui.set_min_content_box_height(decoder->min_ui_height());
        gui.waterfall().set_decoder(decoder);
        decoder->load_ui_settings();
        gui.update_decoder();
    }

    if (s_options.read_stdin) {
        s_log.info() << "Using StdinThread";
        s_processing_thread = std::make_unique<Dsp::StdinThread>(
            decoder,
            s_options.input_sample_rate,
            decoder->input_sample_rate(),
            s_options.input_big_endian,
            s_options.samples_size);
    } else {
        auto& line = s_audio_lines[s_audio_line_index];
        s_log.info() << "Using SoundCardThread, input \"" << line.name << ": " << line.description;
        auto thread = std::make_unique<Dsp::SoundCardThread>(decoder, line.name, decoder->input_sample_rate());
        if (!thread->init_soundcard())
            return false;
        s_processing_thread = std::move(thread);
    }

    s_processing_thread->start();
    s_log.info() << "Started";
    if (!Drtd::using_ui())
        puts("Decoder ready.");
    return true;
}

void Drtd::stop_processing() {
    if (s_processing_thread) {
        s_log.info() << "stop_processing()";
        Ui::FirFilterDialog::close_dialog();
        Ui::BiquadFilterDialog::close_dialog();
        Ui::MovingAverageDialog::close_dialog();
        Ui::IQMixerDialog::close_dialog();

        s_processing_thread->request_stop_and_wait();
        auto& decoder = s_decoders[s_active_decoder_index];
        decoder->tear_down();
        if (using_ui())
            decoder->save_ui_settings();

        s_processing_thread.reset();
    }
}

bool Drtd::use_decoder(u8 index) {
    stop_processing();
    return start_processing(index);
}

void print_usage_and_exit(const char* error_message) {
    if (error_message)
        printf("%s\n\n", error_message);

    puts("Usage: drtd [Options] [Decoder settings]");
    puts("Options:");
    puts("    -g, --headless <Decoder>        Headless mode using the specified decoder.\n"
         "                                    Specify none to show available decoders.\n"
         "                                    Leave parameters empty to show available arguments");
    puts("    -i, --input <Device index>      Use specific audio input device. Specify \"-1\" to show all available devices");
    puts("    -s, --stdin <Sample rate>       Read samples directly from stdin sampled using the specified sample rate");
    puts("        --s16                       When reading from stdin: Samples are 16 bits wide, not default 8");
    puts("        --big-endian                When reading from stdin: Endianess of samples > 8 bit is big");
    puts("    -v                              Show debug messages");
    puts("    -h, --help                      Show this help");

    exit(EXIT_SUCCESS);
    assert(false);
}

void parse_enable_log(int argc, char** argv) {
    for (int i = 0; i < argc; ++i) {
        if (!strcmp(argv[i], "-v")) {
            Util::Logger::enable(true);
            break;
        }
    }
}

void parse_options(int argc, char** argv) {
    std::vector<std::string> decoder_args;
    bool collect_decoder_args = false;

    for (int i = 1; i < argc; ++i) {
        auto arg = argv[i];
        bool has_next = i != (argc - 1);

        if (collect_decoder_args) {
            decoder_args.push_back({ arg });
            continue;
        }

        if (!strcmp(arg, "-g") || !strcmp(arg, "--headless")) {
            auto print_available = []() {
                printf("Available headless decoders: ");
                bool print_seperator = false;
                for (auto& decoder : s_decoders) {
                    if (!decoder->headless())
                        continue;

                    if (print_seperator)
                        printf(", ");

                    printf("%s", decoder->name().c_str());
                    print_seperator = true;
                }
                puts("");
            };

            if (!has_next) {
                print_available();
                print_usage_and_exit("Decoder has to be specified!");
            }

            const auto provided = argv[++i];
            u8 search = 0;
            for (; search < s_decoders.size(); ++search) {
                if (Util::to_lower(s_decoders[search]->name()) == Util::to_lower(provided)) {
                    if (!s_decoders[search]->headless()) {
                        print_available();
                        print_usage_and_exit("Specified decoder does not support headless mode!");
                    }

                    break;
                }
            }

            if (search == s_decoders.size()) {
                print_available();
                print_usage_and_exit("Unknown decoder!");
            }

            s_options.headless_decoder_index = search;
            s_options.ui_mode = false;
        } else if (!strcmp(arg, "-h") || !strcmp(arg, "--help")) {
            print_usage_and_exit(nullptr);
        } else if (!strcmp(arg, "--s16")) {
            s_options.samples_size = Dsp::StdinThread::SampleSize::S16;
        } else if (!strcmp(arg, "--big-endian")) {
            s_options.input_big_endian = true;
        } else if (!strcmp(arg, "-i") || !strcmp(arg, "--input")) {
            if (!has_next)
                print_usage_and_exit("Input index has to be specified!");

            char* endptr;
            int8_t index = static_cast<int8_t>(strtol(argv[++i], &endptr, 10));
            if (*endptr == '\0') {
                if (index < 0)
                    s_options.input_index = Options::input_show_available;
                else
                    s_options.input_index = index;
            } else {
                print_usage_and_exit("Invalid index specified!");
            }
        } else if (!strcmp(arg, "-s") || !strcmp(arg, "--stdin")) {
            if (!has_next)
                print_usage_and_exit("Sample rate has to be specified!");

            auto next = argv[++i];
            int sample_rate = atoi(next);
            if (sample_rate <= 0 || sample_rate > 65535)
                print_usage_and_exit("Sample rate is in invalid range!");

            s_options.input_sample_rate = static_cast<SampleRate>(sample_rate);
            s_options.read_stdin = true;
        } else if (strcmp(arg, "-v")) {
            if (s_options.ui_mode) {
                printf("Unrecognized option \"%s\"!\n", arg);
                print_usage_and_exit(nullptr);
            } else {
                collect_decoder_args = true;
                decoder_args.push_back({ arg });
            }
        }
    }

    if (!decoder_args.empty()) {
        s_options.decoder_parameters = Util::Buffer<std::string>(decoder_args.size());
        for (size_t i = 0; i < decoder_args.size(); ++i)
            s_options.decoder_parameters[i] = decoder_args.at(i);
    }
}

void get_available_audio_lines() {
    s_log.info() << "Searching for available audio lines...";
    char** hints;
    int error;
    std::vector<Drtd::AudioLine> lines;

    if ((error = snd_device_name_hint(-1, "pcm", reinterpret_cast<void***>(&hints))) < 0) {
        std::ostringstream sstr;
        sstr << "Failed to fetch device names: " << snd_strerror(error);
        Util::die(sstr.str().c_str());
    }

    char** current = hints;
    while (*current) {
        char* name = snd_device_name_get_hint(*current, "NAME");
        char* description = snd_device_name_get_hint(*current, "DESC");
        char* ioid = snd_device_name_get_hint(*current, "IOID");

        if (name) {
            if (strcmp(name, "null") && (!ioid || !strcmp(ioid, "Input"))) {
                Drtd::AudioLine line;
                auto strd = s_log.info();

                strd << lines.size() << ": Line \"" << name << '"';
                line.index = static_cast<u8>(lines.size());
                line.name = std::string(name);
                if (description) {
                    strd << " descr.: \"" << description << '"';
                    line.description = std::string(description);
                }

                lines.push_back(line);
                if (line.name == s_default_line_name || s_default_audio_line.name.empty())
                    s_default_audio_line = line;
            }
        }

        if (description)
            free(description);

        if (ioid)
            free(ioid);

        if (name)
            free(name);

        ++current;
    }

    if (lines.size()) {
        s_audio_lines = Buffer<Drtd::AudioLine>(lines.size());
        for (size_t i = 0; i < lines.size(); ++i)
            s_audio_lines[i] = lines.at(i);
    }

    snd_device_name_free_hint(reinterpret_cast<void**>(hints));
}

void handle_sigint(int) {
    s_log.info() << "Received SIGINT, stopping...";
    Drtd::stop_processing();
    puts("Bye.");
    exit(0);
}

int main(int argc, char** argv) {
    std::signal(SIGINT, handle_sigint);

    parse_enable_log(argc, argv);
    s_log.info() << "Initializing decoders...";
    s_decoders = make_decoder_buffer(Dsp::Null(),
                                     Dsp::Ax25(),
                                     Dsp::Rtty(),
                                     Dsp::Pocsag(),
                                     Dsp::Dtmf(),
                                     Dsp::Dcf77());

    parse_options(argc, argv);
    Util::Config::setup(argv[0]);
    Util::Config::load_file();

    if (!s_options.read_stdin) {
        get_available_audio_lines();
        if (!s_audio_lines.size())
            Util::die("No recording audio line found!");

        Util::Config::load(s_conf_audioline, s_audio_line_index, s_default_audio_line.index);

        auto input_index = s_options.input_index;
        if (input_index == Options::input_none_specified) {
            if (s_audio_line_index >= s_audio_lines.size())
                s_audio_line_index = s_default_audio_line.index;
        } else if (input_index == Options::input_show_available) {
            printf("Available inputs:\n");
            for (const auto& line : s_audio_lines)
                printf("\t%d: %s, %s\n", line.index, line.name.c_str(), line.description.c_str());

            exit(EXIT_SUCCESS);
            assert(false);
        } else {
            if (input_index >= static_cast<int8_t>(s_audio_lines.size()))
                print_usage_and_exit("Invalid input index specified!");
            s_audio_line_index = input_index;
        }

        s_log.info() << "Using audio line with index " << static_cast<int>(s_audio_line_index) << ": " << s_audio_lines[s_audio_line_index].name;
    }

    int result = 0;
    if (s_options.ui_mode) {
        s_log.info() << "Starting in GUI mode";
        Fl::lock(); //Enable fltk multi thread locking

        Ui::MainGui::WindowProperties properties;
        properties.position = { 0, 0 };
        properties.size = { Ui::MainGui::default_window_width, Ui::MainGui::default_window_height };

        Util::Config::load(s_conf_decoder_index, s_active_decoder_index, static_cast<u8>(0));
        Util::Config::load(s_conf_main_window, properties, properties);

        Ui::MainGui main_gui(s_active_decoder_index, properties);
        s_main_gui = &main_gui;
        s_main_gui->show();

        if (!Drtd::use_decoder(s_active_decoder_index)) {
            s_log.warning() << "Could not start previous decoder, trying default audio line";
            if (!Drtd::switch_audio_line(Drtd::default_audio_line().index))
                Util::die("Could not start decoder. Outdated config file?");
        }

        result = Fl::run();

        Drtd::stop_processing();
        properties.position = { main_gui.x(), main_gui.y() };
        properties.size = { static_cast<u16>(main_gui.w()), static_cast<u16>(main_gui.h()) };

        Util::Config::save(s_conf_decoder_index, s_active_decoder_index);
        Util::Config::save(s_conf_main_window, properties);
        Util::Config::save(s_conf_audioline, s_audio_line_index);
        Util::Config::save_all();
    } else {
        s_log.info() << "Starting in headless mode";
        auto& decoder = s_decoders[s_options.headless_decoder_index];
        auto available_params = decoder->changeable_parameters();
        auto print_params_and_exit = [](const Util::Buffer<std::string>& params, const char* error) {
            printf("Invalid decoder settings!\nAvailable settings: ");
            if (params.size()) {
                putchar('[');
                bool print_seperator = false;

                for (auto& str : params) {
                    if (print_seperator)
                        printf("] [");

                    printf("%s", str.c_str());
                    print_seperator = true;
                }

                puts("]");
            } else {
                puts("None");
            }

            print_usage_and_exit(error);
        };

        if (available_params.size() != s_options.decoder_parameters.size())
            print_params_and_exit(available_params, "Settings count mismatch!");

        if (!decoder->setup_parameters(s_options.decoder_parameters))
            print_params_and_exit(available_params, "Invalid settings value provided!");

        if (!Drtd::start_processing(s_options.headless_decoder_index))
            Util::die("Could not start decoder! Outdated save file? Unsupported input device?");

        s_log.info() << "Joining processing thread";
        s_processing_thread->join();
    }

    return result;
}
