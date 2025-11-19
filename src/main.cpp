#include "cpp_xtemplate/cpp_xtemplate.hpp"
#include "cpp_xtemplate/xtemplate_hardcoded.h"
#include <gdkmm.h>
#include <glibmm.h>
#include <gtkmm.h>

class XGtkmm3Template : public Gtk::Window {
    public:
        XGtkmm3Template() {
            setup_ui();
            setup_signals();
            setup_data();

            show_all_children();
        }

    protected:
        void render_template(const std::vector<Gtk::Entry *> &entries_vars,
                             const std::string &body, std::string &result) {
            std::string new_text = body;
            int var_num = 1;
            for (const Gtk::Entry *entry_var : entries_vars) {
                std::string replacement = entry_var->get_text();
                if (replacement.empty()) {
                    var_num++;
                    continue;
                }
                std::string placeholder = "$" + std::to_string(var_num);
                size_t pos = 0;
                while ((pos = new_text.find(placeholder, pos)) !=
                       std::string::npos) {
                    new_text.replace(pos, placeholder.length(), replacement);
                    pos += replacement.length();
                }
                var_num++;
            }
            result = new_text;
        }

        void show_notification(Gtk::Window &parent,
                               const Glib::ustring &message) {
            auto screen = parent.get_screen();
            Gdk::Rectangle rect;
            screen->get_monitor_geometry(screen->get_primary_monitor(), rect);

            auto *notif = new Gtk::Window(Gtk::WINDOW_POPUP);
            notif->set_transient_for(parent);
            notif->set_attached_to(parent);
            notif->set_screen(parent.get_screen());
            notif->set_type_hint(Gdk::WINDOW_TYPE_HINT_NOTIFICATION);
            notif->set_skip_taskbar_hint(true);
            notif->set_skip_pager_hint(true);
            notif->set_decorated(false);
            notif->set_opacity(0.95);

            auto *frame = Gtk::make_managed<Gtk::Frame>();
            frame->set_shadow_type(Gtk::SHADOW_ETCHED_IN);
            frame->override_background_color(Gdk::RGBA("#f0f0f000"));
            auto *label = Gtk::make_managed<Gtk::Label>(message);
            label->override_color(Gdk::RGBA("#000000"));
            label->set_line_wrap(true);
            label->set_margin_start(12);
            label->set_margin_end(12);
            label->set_margin_top(8);
            label->set_margin_bottom(8);
            frame->add(*label);
            notif->add(*frame);
            notif->show_all();

            int w = 350, h = 60;
            notif->set_default_size(w, h);
            notif->get_size(w, h);
            notif->move(rect.get_x() + rect.get_width() - w - 20,
                        rect.get_y() + rect.get_height() - h - 60);
            Glib::signal_timeout().connect_seconds(
                [notif]() {
                    notif->hide();
                    delete notif;
                    return false;
                },
                4);
        }

        bool on_exit_clicked() {
            int ret =
                write_config(config_path, liststore_templates, template_cols);
            if (ret == 0)
                return false;
            Gtk::MessageDialog dialog(
                *this,
                "Error with saving templates: errno = " + std::to_string(ret),
                false, Gtk::MESSAGE_INFO, Gtk::BUTTONS_OK, true);
            dialog.run();
            return true;
        }

        void on_template_selection_changed() {
            Gtk::TreeModel::iterator curr_template =
                treeview_templates.get_selection()->get_selected();
            if (!curr_template)
                return;
            Gtk::TreeModel::iterator true_curr_template =
                filter_templates->convert_iter_to_child_iter(*curr_template);

            std::vector<std::string> tags =
                (*true_curr_template)[template_cols.tags];
            Glib::ustring tags_str;
            for (size_t i = 0; i < tags.size(); ++i) {
                if (i > 0)
                    tags_str += ", ";
                tags_str += tags[i];
            }
            label_tags.set_text(tags_str);
        }

        void on_open_template_clicked() {
            Gtk::TreeModel::iterator curr_template =
                treeview_templates.get_selection()->get_selected();
            if (!curr_template)
                return;
            if (app_state == STATE_TEMPLATE_OPENED) {
                stack_main.set_visible_child(frame_templates);
                app_state = STATE_SEARCH;
                return;
            }
            app_state = STATE_TEMPLATE_OPENED;

            if (textview_template_result.get_parent())
                textview_template_result.unparent();
            for (Gtk::Widget *child : vbox_template_form.get_children()) {
                vbox_template_form.remove(*child);
            }
            Gtk::TreeModel::Row true_curr_row =
                *filter_templates->convert_iter_to_child_iter(*curr_template);
            std::vector<std::string> vars_types =
                true_curr_row[template_cols.vars_types];
            std::vector<std::string> vars_names =
                true_curr_row[template_cols.vars_names];
            std::vector<Gtk::Entry *> entries_vars;
            Gtk::Grid *grid_vars = Gtk::make_managed<Gtk::Grid>();
            grid_vars->set_column_spacing(10);
            grid_vars->set_row_spacing(10);
            vbox_template_form.pack_start(*grid_vars, Gtk::PACK_SHRINK);
            size_t row_idx = 0;
            for (size_t i = 0; i < vars_names.size(); ++i) {
                Gtk::Frame *frame_var_type = Gtk::make_managed<Gtk::Frame>();
                Gtk::Label *label_var_type = Gtk::make_managed<Gtk::Label>();
                set_margin(*label_var_type, 5, 0);
                frame_var_type->add(*label_var_type);
                grid_vars->attach(*frame_var_type, 0, row_idx, 1, 1);
                label_var_type->set_halign(Gtk::ALIGN_START);
                label_var_type->set_text(vars_types[i]);
                Gtk::Label *label_var_name = Gtk::make_managed<Gtk::Label>();
                grid_vars->attach(*label_var_name, 1, row_idx, 1, 1);
                label_var_name->set_halign(Gtk::ALIGN_START);
                label_var_name->set_text(vars_names[i]);
                Gtk::Entry *entry_var = Gtk::make_managed<Gtk::Entry>();
                grid_vars->attach(*entry_var, 2, row_idx, 1, 1);
                entry_var->set_text("");
                entries_vars.push_back(entry_var);
                row_idx++;
            }

            Gtk::Button *button_result = Gtk::make_managed<Gtk::Button>();
            button_result->set_label("Copy");
            vbox_template_form.pack_start(*button_result, Gtk::PACK_SHRINK);
            Gtk::Frame *frame_result = Gtk::make_managed<Gtk::Frame>();
            vbox_template_form.pack_start(*frame_result,
                                          Gtk::PACK_EXPAND_WIDGET);
            Gtk::ScrolledWindow *scrolled_result =
                Gtk::make_managed<Gtk::ScrolledWindow>();
            frame_result->add(*scrolled_result);
            scrolled_result->set_name("white_background");
            scrolled_result->add(textview_template_result);

            Glib::RefPtr<Gtk::TextBuffer> buffer =
                textview_template_result.get_buffer();
            buffer->set_text(
                static_cast<Glib::ustring>(true_curr_row[template_cols.body]));
            button_result->signal_clicked().connect(
                [this, buffer, entries_vars, true_curr_row]() {
                    std::string result;
                    render_template(entries_vars,
                                    static_cast<Glib::ustring>(
                                        true_curr_row[template_cols.body]),
                                    result);
                    buffer->set_text(result);
                    std::string cmd = "xclip -selection clipboard";
                    FILE *pipe = popen(cmd.c_str(), "w");
                    if (pipe) {
                        fwrite(result.c_str(), 1, result.size(), pipe);
                        pclose(pipe);
                        show_notification(*this,
                                          "Successfylly copied to clipboard");
                    }
                });
            set_margin(textview_template_result, 10, 10);
            vbox_template_form.show_all();
            stack_main.set_visible_child(vbox_template_form);
        }

        void on_new_template_clicked() {
            if (app_state == STATE_TEMPLATE_CREATION) {
                stack_main.set_visible_child(frame_templates);
                app_state = STATE_SEARCH;
                return;
            }
            app_state = STATE_TEMPLATE_CREATION;
            for (Gtk::Widget *child : vbox_new_template_form.get_children()) {
                vbox_new_template_form.remove(*child);
            }

            Gtk::Grid *grid_name_tags = Gtk::make_managed<Gtk::Grid>();
            grid_name_tags->set_column_spacing(10);
            grid_name_tags->set_row_spacing(10);
            vbox_new_template_form.pack_start(*grid_name_tags,
                                              Gtk::PACK_SHRINK);
            Gtk::Label *label_new_name = Gtk::make_managed<Gtk::Label>();
            grid_name_tags->attach(*label_new_name, 0, 0, 1, 1);
            label_new_name->set_text("Name");
            Gtk::Entry *entry_new_name = Gtk::make_managed<Gtk::Entry>();
            grid_name_tags->attach(*entry_new_name, 1, 0, 1, 1);
            entry_new_name->set_text("");
            Gtk::Label *label_new_tags = Gtk::make_managed<Gtk::Label>();
            grid_name_tags->attach(*label_new_tags, 0, 1, 1, 1);
            label_new_tags->set_text("Tags");
            Gtk::Entry *entry_new_tags = Gtk::make_managed<Gtk::Entry>();
            grid_name_tags->attach(*entry_new_tags, 1, 1, 1, 1);
            entry_new_tags->set_tooltip_text("Tags comma-separated");
            entry_new_tags->set_text("");
            Gtk::Label *label_new_vars = Gtk::make_managed<Gtk::Label>();
            vbox_new_template_form.pack_start(*label_new_vars,
                                              Gtk::PACK_SHRINK);
            label_new_vars->set_text("Variables' names");
            Gtk::Grid *grid_new_vars = Gtk::make_managed<Gtk::Grid>();
            grid_new_vars->set_column_spacing(10);
            grid_new_vars->set_row_spacing(10);
            vbox_new_template_form.pack_start(*grid_new_vars, Gtk::PACK_SHRINK);
            Gtk::Button *button_new_var = Gtk::make_managed<Gtk::Button>();
            button_new_var->set_label("Append variable");
            button_new_var->set_margin_top(20);
            vbox_new_template_form.pack_start(*button_new_var,
                                              Gtk::PACK_SHRINK);
            button_new_var->signal_clicked().connect([this, grid_new_vars]() {
                int next_var_num = 0;
                while (grid_new_vars->get_child_at(1, next_var_num)) {
                    Gtk::Widget *child =
                        grid_new_vars->get_child_at(1, next_var_num);
                    if (!child)
                        break;
                    next_var_num++;
                }

                Gtk::Label *label_var_name = Gtk::make_managed<Gtk::Label>();
                grid_new_vars->attach(*label_var_name, 0, next_var_num, 1, 1);
                label_var_name->set_halign(Gtk::ALIGN_START);
                label_var_name->set_text("$" +
                                         std::to_string(next_var_num + 1));
                Gtk::Entry *entry_var = Gtk::make_managed<Gtk::Entry>();
                entry_var->set_text("");
                grid_new_vars->attach(*entry_var, 1, next_var_num, 1, 1);
                grid_new_vars->show_all();
            });
            Gtk::Label *label_new_body = Gtk::make_managed<Gtk::Label>();
            label_new_body->set_text("Body");
            vbox_new_template_form.pack_start(*label_new_body,
                                              Gtk::PACK_SHRINK);
            Gtk::Frame *frame_new_body = Gtk::make_managed<Gtk::Frame>();
            vbox_new_template_form.pack_start(*frame_new_body,
                                              Gtk::PACK_EXPAND_WIDGET);
            Gtk::ScrolledWindow *scrolled_new_body =
                Gtk::make_managed<Gtk::ScrolledWindow>();
            frame_new_body->add(*scrolled_new_body);
            scrolled_new_body->set_name("white_background");
            Gtk::TextView *textview_new_body =
                Gtk::make_managed<Gtk::TextView>();
            scrolled_new_body->add(*textview_new_body);
            textview_new_body->set_editable(true);

            Gtk::Button *button_create_template =
                Gtk::make_managed<Gtk::Button>();
            vbox_new_template_form.pack_start(*button_create_template,
                                              Gtk::PACK_SHRINK);
            button_create_template->set_label("Create");
            button_create_template->set_sensitive(false);
            entry_new_name->signal_changed().connect(
                [this, button_create_template, entry_new_name,
                 textview_new_body]() {
                    button_create_template->set_sensitive(
                        !entry_new_name->get_text().empty() &&
                        !textview_new_body->get_buffer()->get_text().empty());
                });
            textview_new_body->get_buffer()->signal_changed().connect(
                [this, button_create_template, entry_new_name,
                 textview_new_body]() {
                    button_create_template->set_sensitive(
                        !textview_new_body->get_buffer()->get_text().empty() &&
                        !entry_new_name->get_text().empty());
                });
            button_create_template->signal_clicked().connect(
                [this, grid_new_vars, entry_new_name, entry_new_tags,
                 textview_new_body]() {
                    std::vector<std::string> vars_names;
                    int row = 0;
                    while (grid_new_vars->get_child_at(1, row)) {
                        Gtk::Widget *child =
                            grid_new_vars->get_child_at(1, row);
                        if (child) {
                            Gtk::Entry *entry =
                                dynamic_cast<Gtk::Entry *>(child);
                            if (entry)
                                vars_names.push_back(entry->get_text());
                        }
                        ++row;
                    }
                    Gtk::TreeModel::Row new_row =
                        *(liststore_templates->append());
                    new_row[template_cols.name] = entry_new_name->get_text();
                    new_row[template_cols.vars_names] = vars_names;
                    new_row[template_cols.tags] =
                        split_by_comma(entry_new_tags->get_text());
                    new_row[template_cols.body] =
                        textview_new_body->get_buffer()->get_text();
                    treeview_templates.get_selection()->select(
                        filter_templates->children().begin());
                    app_state = STATE_SEARCH;
                    stack_main.set_visible_child(frame_templates);
                });
            set_margin(*textview_new_body, 10, 10);
            label_new_vars->set_margin_top(20);
            label_new_body->set_margin_top(20);
            vbox_new_template_form.show_all();
            stack_main.set_visible_child(vbox_new_template_form);
        }

        void on_delete_template_clicked() {
            if (app_state != STATE_SEARCH) {
                stack_main.set_visible_child(frame_templates);
                app_state = STATE_SEARCH;
                return;
            }
            Gtk::TreeModel::iterator curr_template =
                treeview_templates.get_selection()->get_selected();
            if (!curr_template)
                return;
            Gtk::MessageDialog dialog("Are u sure?", false,
                                      Gtk::MESSAGE_QUESTION,
                                      Gtk::BUTTONS_YES_NO, true);
            dialog.set_default_response(Gtk::RESPONSE_NO);
            if (dialog.run() == Gtk::RESPONSE_YES) {
                Gtk::TreeModel::iterator true_curr_template =
                    filter_templates->convert_iter_to_child_iter(
                        *curr_template);
                liststore_templates->erase(true_curr_template);
                if (liststore_templates->children())
                    treeview_templates.get_selection()->select(
                        filter_templates->children().begin());
            }
        }

        bool
        filter_templates_by_tag(const Gtk::TreeModel::const_iterator &iter) {
            if (!iter)
                return false;
            if (app_state != STATE_SEARCH)
                on_open_template_clicked();
            const std::vector<std::string> &tags = (*iter)[template_cols.tags];
            Glib::ustring search_text = entry_search_tag.get_text();
            if (search_text.empty())
                return true;
            auto regex = Glib::Regex::create(",\\s*");
            std::vector<Glib::ustring> searched_tags =
                regex->split(search_text);
            for (auto &search_tag : searched_tags) {
                Glib::ustring trimmed_tag = trim(search_tag);
                if (trimmed_tag.empty())
                    continue;
                bool found = false;
                for (const auto &tag : tags) {
                    if (Glib::ustring(tag).find(trimmed_tag) !=
                        Glib::ustring::npos) {
                        found = true;
                        break;
                    }
                }
                if (!found)
                    return false;
            }
            return true;
        }

    private:
        void setup_ui() {
            set_title("XTemplate");
            auto css = Gtk::CssProvider::create();
            css->load_from_data(R"(
                * {
                    font-family: Sans;
                    font-size: 14px;
                }
                #white_background{
                    background-image: none;
                    background-color: white;
                }
            )");
            auto screen = get_screen();
            Gtk::StyleContext::add_provider_for_screen(
                screen, css, GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

            add(vbox_main);
            vbox_main.set_hexpand(true);
            vbox_main.set_vexpand(true);
            set_size_request(800, 600);

            header_bar.set_show_close_button(true);
            header_bar.set_title("XTemplate");
            header_bar.pack_start(button_open_template);
            set_titlebar(header_bar);
            button_open_template.set_label("Open Template");
            set_margin(button_open_template, 5, 0);
            header_bar.pack_start(button_new_template);
            button_new_template.set_label("New Template");
            set_margin(button_new_template, 5, 0);
            header_bar.pack_start(button_delete_template);
            button_delete_template.set_label("Delete Template");
            set_margin(button_delete_template, 5, 0);

            vbox_main.pack_start(stack_main, Gtk::PACK_EXPAND_WIDGET);
            stack_main.add(frame_templates);
            stack_main.set_homogeneous(true);
            frame_templates.add(scrolled_templates);
            scrolled_templates.add(treeview_templates);
            scrolled_templates.set_name("white_background");
            scrolled_templates.set_hexpand(true);
            scrolled_templates.set_vexpand(true);

            stack_main.add(vbox_template_form);
            textview_template_result.set_editable(false);
            stack_main.add(vbox_new_template_form);
            frame_templates.show_all();
            stack_main.set_visible_child(frame_templates);

            vbox_main.pack_start(hbox_bottom, false, true);
            hbox_bottom.pack_start(label_tag, false, true);
            label_tag.set_text("Tags: ");
            hbox_bottom.pack_start(label_tags, false, true);
            hbox_bottom.pack_start(entry_search_tag, true, true);
            entry_search_tag.set_placeholder_text(
                "Searched tags comma-separated");
            hbox_bottom.pack_start(button_search_tag, false, true);
            button_search_tag.set_label("Search");
            set_margin(vbox_main, 10, 10);
            set_margin(frame_templates, 10, 0);
            set_margin(treeview_templates, 10, 10);
            set_margin(vbox_template_form, 10, 10);
            set_margin(hbox_bottom, 10, 5);
        }

        void setup_signals() {
            signal_delete_event().connect([this](GdkEventAny *event) {
                bool ret = on_exit_clicked();
                return ret;
            });
            treeview_templates.get_selection()->signal_changed().connect(
                sigc::mem_fun(*this,
                              &XGtkmm3Template::on_template_selection_changed));
            treeview_templates.signal_row_activated().connect(
                [this](const Gtk::TreePath &path, Gtk::TreeViewColumn *column) {
                    on_open_template_clicked();
                });
            button_search_tag.signal_clicked().connect(
                [this]() { filter_templates->refilter(); });
            button_open_template.signal_clicked().connect(
                [this]() { on_open_template_clicked(); });
            button_new_template.signal_clicked().connect(
                [this]() { on_new_template_clicked(); });
            button_delete_template.signal_clicked().connect(
                [this]() { on_delete_template_clicked(); });
        }

        void setup_data() {
            liststore_templates = Gtk::ListStore::create(template_cols);
            filter_templates =
                Gtk::TreeModelFilter::create(liststore_templates);
            filter_templates->set_visible_func(sigc::mem_fun(
                *this, &XGtkmm3Template::filter_templates_by_tag));
            treeview_templates.set_model(filter_templates);
            treeview_templates.append_column("Name", template_cols.name);

            parse_config_hardcoded(xtemplate_hardcoded, liststore_templates,
                                   template_cols);
            parse_config(config_path, liststore_templates, template_cols);

            treeview_templates.get_selection()->select(
                filter_templates->children().begin());
        }

        Gtk::HeaderBar header_bar;
        Gtk::Box vbox_main{Gtk::ORIENTATION_VERTICAL, 10};
        Gtk::Button button_open_template;
        Gtk::Stack stack_main;
        Gtk::Frame frame_templates;
        Gtk::ScrolledWindow scrolled_templates;
        Gtk::TreeView treeview_templates;
        Gtk::Box vbox_template_form{Gtk::ORIENTATION_VERTICAL, 10};
        Gtk::Box vbox_new_template_form{Gtk::ORIENTATION_VERTICAL, 10};
        Gtk::TextView textview_template_result;
        Gtk::Box hbox_bottom{Gtk::ORIENTATION_HORIZONTAL, 10};
        Gtk::Button button_search_tag, button_new_template,
            button_delete_template;
        Gtk::Entry entry_search_tag;
        Gtk::Label label_tag;
        Gtk::Label label_tags;
        int app_state = STATE_SEARCH;
        std::string config_path =
            std::string(getenv("HOME")) + "/.config/xtemplate.txt";

        Glib::RefPtr<Gtk::TreeModelFilter> filter_templates;
        Glib::RefPtr<Gtk::ListStore> liststore_templates;
        TemplateCols template_cols;
};

int main(int argc, char **argv) {
    auto app = Gtk::Application::create(argc, argv, "github.x-gtkmm3-template");
    XGtkmm3Template window;
    return app->run(window);
}
