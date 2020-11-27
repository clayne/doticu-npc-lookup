/*
    Copyright � 2020 r-neal-kelly, aka doticu
*/

#pragma once

#include "mcm_main.h"

namespace doticu_npcl { namespace MCM {

    template <typename Base_t, typename Item_t>
    class Selectable_Data_t
    {
    public:
        using Filter_t          = std::remove_pointer_t<decltype(Base_t::Self()->Filter())>;
        using Smart_Get_f       = String_t(Filter_t::*)();
        using Smart_Set_f       = void(Filter_t::*)(String_t);

        using Select_f          = void(*)(Item_t, Vector_t<String_t>&);
        using Select_Relation_f = void(*)(Item_t, Actor_Base_t*, Vector_t<String_t>&);

    public:
        Vector_t<Item_t>    items;
        Vector_t<String_t>  results;
        Vector_t<String_t>  buffer;

        Selectable_Data_t(Smart_Get_f smart_get_f,
                          Smart_Set_f smart_set_f,
                          Select_f select_f)
        {
            Init(smart_get_f, smart_set_f);
            Select(select_f);
            Sort();
        }

        Selectable_Data_t(Smart_Get_f smart_get_f,
                          Smart_Set_f smart_set_f,
                          Select_Relation_f select_f,
                          Actor_Base_t* base_to_compare)
        {
            Init(smart_get_f, smart_set_f);
            Select(select_f, base_to_compare);
            Sort();
        }

        inline void Init(Smart_Get_f smart_get_f, Smart_Set_f smart_set_f)
        {
            auto* self = Base_t::Self();
            auto* list = self->List();
            auto* filter = self->Filter();
            auto* options = self->Options();

            if (options->Do_Smart_Select()) {
                String_t current = (filter->*smart_get_f)();
                (filter->*smart_set_f)("");
                list->do_update_items = true;
                items = list->Items();
                (filter->*smart_set_f)(current);
            } else {
                items = list->Default_Items();
            }

            size_t item_count = items.size();
            if (item_count > 2048) {
                results.reserve(2048);
            } else {
                results.reserve(item_count);
            }
            buffer.reserve(128);
        }

        inline void Select(Select_f select_f)
        {
            results.push_back(" Any ");
            for (Index_t idx = 0, end = items.size(); idx < end; idx += 1) {
                Item_t item = items.at(idx);
                if (item) {
                    select_f(item, buffer);
                    for (Index_t idx = 0, end = buffer.size(); idx < end; idx += 1) {
                        String_t string = buffer[idx];
                        if (string && !results.Has(string)) {
                            results.push_back(string);
                        }
                    }
                    buffer.clear();
                }
            }
        }

        inline void Select(Select_Relation_f select_f, Actor_Base_t* base_to_compare)
        {
            results.push_back(" Any ");
            for (Index_t idx = 0, end = items.size(); idx < end; idx += 1) {
                Item_t item = items.at(idx);
                if (item) {
                    select_f(item, base_to_compare, buffer);
                    for (Index_t idx = 0, end = buffer.size(); idx < end; idx += 1) {
                        String_t string = buffer[idx];
                        if (string && !results.Has(string)) {
                            results.push_back(string);
                        }
                    }
                    buffer.clear();
                }
            }
        }

        inline void Sort()
        {
            static auto comparator = [](String_t* str_a, String_t* str_b)->Int_t
            {
                return Main_t::String_Comparator(
                    str_a ? str_a->data : "",
                    str_b ? str_b->data : ""
                );
            };
            results.Sort(comparator, 1);
        }

        inline Vector_t<String_t> Results()
        {
            return results;
        }
    };

    template <typename Base_t, typename Item_t>
    class Selectable_Mods_i : public Selectable_Data_t<Base_t, Item_t>
    {
    public:
        Selectable_Mods_i(Select_f select_f) :
            Selectable_Data_t<Base_t, Item_t>(&Filter_t::Mod_Argument, &Filter_t::Mod_Argument, select_f)
        {
        }
    };

    template <typename Base_t, typename Item_t>
    class Selectable_Mods_t : public Selectable_Mods_i<Base_t, Item_t>
    {
    };

    template <typename Base_t>
    class Selectable_Mods_t<Base_t, Form_t*> : public Selectable_Mods_i<Base_t, Form_t*>
    {
    public:
        using Item_t = Form_t*;

    public:
        Selectable_Mods_t() :
            Selectable_Mods_i<Base_t, Item_t>(&Select)
        {
        }

        static void Select(Item_t item, Vector_t<String_t>& output)
        {
            if (item && item->Is_Valid()) {
                item->Mod_Names(output);
            }
        }
    };

    template <typename Base_t>
    class Selectable_Mods_t<Base_t, Actor_Base_t*> : public Selectable_Mods_i<Base_t, Actor_Base_t*>
    {
    public:
        using Item_t = Actor_Base_t*;

    public:
        Selectable_Mods_t() :
            Selectable_Mods_i<Base_t, Item_t>(&Select)
        {
        }

        static void Select(Item_t item, Vector_t<String_t>& output)
        {
            Selectable_Mods_t<Base_t, Form_t*>::Select(item, output);
        }
    };

    template <typename Base_t>
    class Selectable_Mods_t<Base_t, Leveled_Actor_Base_t*> : public Selectable_Mods_i<Base_t, Leveled_Actor_Base_t*>
    {
    public:
        using Item_t = Leveled_Actor_Base_t*;

    public:
        Selectable_Mods_t() :
            Selectable_Mods_i<Base_t, Item_t>(&Select)
        {
        }

        static void Select(Item_t item, Vector_t<String_t>& output)
        {
            Selectable_Mods_t<Base_t, Form_t*>::Select(item, output);
        }
    };

    template <typename Base_t>
    class Selectable_Mods_t<Base_t, Actor_t*> : public Selectable_Mods_i<Base_t, Actor_t*>
    {
    public:
        using Item_t = Actor_t*;

    public:
        Selectable_Mods_t() :
            Selectable_Mods_i<Base_t, Item_t>(&Select)
        {
        }

        static void Select(Item_t item, Vector_t<String_t>& output)
        {
            if (item && item->Is_Valid()) {
                Selectable_Mods_t<Base_t, Form_t*>::Select(item, output);
            }
        }
    };

    template <typename Base_t>
    class Selectable_Mods_t<Base_t, Loaded_Actor_t> : public Selectable_Mods_i<Base_t, Loaded_Actor_t>
    {
    public:
        using Item_t = Loaded_Actor_t;

    public:
        Selectable_Mods_t() :
            Selectable_Mods_i<Base_t, Item_t>(&Select)
        {
        }

        static void Select(Item_t item, Vector_t<String_t>& output)
        {
            if (item.Is_Valid()) {
                Selectable_Mods_t<Base_t, Form_t*>::Select(item.actor, output);
            }
        }
    };

    template <typename Base_t, typename Item_t>
    class Selectable_Races_i : public Selectable_Data_t<Base_t, Item_t>
    {
    public:
        Selectable_Races_i(Select_f select_f) :
            Selectable_Data_t<Base_t, Item_t>(&Filter_t::Race_Argument, &Filter_t::Race_Argument, select_f)
        {
        }
    };

    template <typename Base_t, typename Item_t>
    class Selectable_Races_t : public Selectable_Races_i<Base_t, Item_t>
    {
    };

    template <typename Base_t>
    class Selectable_Races_t<Base_t, Actor_Base_t*> : public Selectable_Races_i<Base_t, Actor_Base_t*>
    {
    public:
        using Item_t = Actor_Base_t*;

    public:
        Selectable_Races_t() :
            Selectable_Races_i<Base_t, Item_t>(&Select)
        {
        }

        static void Select(Item_t item, Vector_t<String_t>& output)
        {
            Race_t* race = item->Race();
            if (race) {
                output.push_back(race->Get_Editor_ID());
            } else {
                output.push_back("");
            }
        }
    };

    template <typename Base_t>
    class Selectable_Races_t<Base_t, Leveled_Actor_Base_t*> : public Selectable_Races_i<Base_t, Leveled_Actor_Base_t*>
    {
    public:
        using Item_t = Leveled_Actor_Base_t*;

    public:
        Selectable_Races_t() :
            Selectable_Races_i<Base_t, Item_t>(&Select)
        {
        }

        static void Select(Item_t item, Vector_t<String_t>& output)
        {
            item->Race_Names(output);
        }
    };

    template <typename Base_t>
    class Selectable_Races_t<Base_t, Actor_t*> : public Selectable_Races_i<Base_t, Actor_t*>
    {
    public:
        using Item_t = Actor_t*;

    public:
        Selectable_Races_t() :
            Selectable_Races_i<Base_t, Item_t>(&Select)
        {
        }

        static void Select(Item_t item, Vector_t<String_t>& output)
        {
            if (item && item->Is_Valid()) {
                Selectable_Races_t<Base_t, Actor_Base_t*>::Select(item->Actor_Base(), output);
            }
        }
    };

    template <typename Base_t>
    class Selectable_Races_t<Base_t, Loaded_Actor_t> : public Selectable_Races_i<Base_t, Loaded_Actor_t>
    {
    public:
        using Item_t = Loaded_Actor_t;

    public:
        Selectable_Races_t() :
            Selectable_Races_i<Base_t, Item_t>(&Select)
        {
        }

        static void Select(Item_t item, Vector_t<String_t>& output)
        {
            if (item.Is_Valid()) {
                Selectable_Races_t<Base_t, Actor_Base_t*>::Select(item.actor->Actor_Base(), output);
            }
        }
    };

    template <typename Base_t, typename Item_t>
    class Selectable_Bases_i : public Selectable_Data_t<Base_t, Item_t>
    {
    public:
        Selectable_Bases_i(Select_f select_f) :
            Selectable_Data_t<Base_t, Item_t>(&Filter_t::Base_Argument, &Filter_t::Base_Argument, select_f)
        {
        }
    };

    template <typename Base_t, typename Item_t>
    class Selectable_Bases_t : public Selectable_Bases_i<Base_t, Item_t>
    {
    };

    template <typename Base_t>
    class Selectable_Bases_t<Base_t, Actor_Base_t*> : public Selectable_Bases_i<Base_t, Actor_Base_t*>
    {
    public:
        using Item_t = Actor_Base_t*;

    public:
        Selectable_Bases_t() :
            Selectable_Bases_i<Base_t, Item_t>(&Select)
        {
        }

        static void Select(Item_t item, Vector_t<String_t>& output)
        {
            output.push_back(item->Any_Name());
        }
    };

    template <typename Base_t>
    class Selectable_Bases_t<Base_t, Leveled_Actor_Base_t*> : public Selectable_Bases_i<Base_t, Leveled_Actor_Base_t*>
    {
    public:
        using Item_t = Leveled_Actor_Base_t*;

    public:
        Selectable_Bases_t() :
            Selectable_Bases_i<Base_t, Item_t>(&Select)
        {
        }

        static void Select(Item_t item, Vector_t<String_t>& output)
        {
            item->Actor_Base_Names(output);
        }
    };

    template <typename Base_t>
    class Selectable_Bases_t<Base_t, Actor_t*> : public Selectable_Bases_i<Base_t, Actor_t*>
    {
    public:
        using Item_t = Actor_t*;

    public:
        Selectable_Bases_t() :
            Selectable_Bases_i<Base_t, Item_t>(&Select)
        {
        }

        static void Select(Item_t item, Vector_t<String_t>& output)
        {
            if (item && item->Is_Valid()) {
                Selectable_Bases_t<Base_t, Actor_Base_t*>::Select(item->Actor_Base(), output);
            }
        }
    };

    template <typename Base_t>
    class Selectable_Bases_t<Base_t, Loaded_Actor_t> : public Selectable_Bases_i<Base_t, Loaded_Actor_t>
    {
    public:
        using Item_t = Loaded_Actor_t;

    public:
        Selectable_Bases_t() :
            Selectable_Bases_i<Base_t, Item_t>(&Select)
        {
        }

        static void Select(Item_t item, Vector_t<String_t>& output)
        {
            if (item.Is_Valid()) {
                Selectable_Bases_t<Base_t, Actor_Base_t*>::Select(item.actor->Actor_Base(), output);
            }
        }
    };

    template <typename Base_t, typename Item_t>
    class Selectable_Templates_i : public Selectable_Data_t<Base_t, Item_t>
    {
    public:
        Selectable_Templates_i(Select_f select_f) :
            Selectable_Data_t<Base_t, Item_t>(&Filter_t::Template_Argument, &Filter_t::Template_Argument, select_f)
        {
        }
    };

    template <typename Base_t, typename Item_t>
    class Selectable_Templates_t : public Selectable_Templates_i<Base_t, Item_t>
    {
    };

    template <typename Base_t>
    class Selectable_Templates_t<Base_t, Actor_Base_t*> : public Selectable_Templates_i<Base_t, Actor_Base_t*>
    {
    public:
        using Item_t = Actor_Base_t*;

    public:
        Selectable_Templates_t() :
            Selectable_Templates_i<Base_t, Item_t>(&Select)
        {
        }

        static void Select(Item_t item, Vector_t<String_t>& output)
        {
            if (item && item->Is_Valid()) {
                for (Actor_Base_t* it = item->template_list; it != nullptr && it->Is_Valid(); it = it->template_list) {
                    String_t name = it->Any_Name();
                    if (name && !output.Has(name)) {
                        output.push_back(name);
                    }
                }
            }
        }
    };

    template <typename Base_t>
    class Selectable_Templates_t<Base_t, Leveled_Actor_Base_t*> : public Selectable_Templates_i<Base_t, Leveled_Actor_Base_t*>
    {
    public:
        using Item_t = Leveled_Actor_Base_t*;

    public:
        Selectable_Templates_t() :
            Selectable_Templates_i<Base_t, Item_t>(&Select)
        {
        }

        static void Select(Item_t item, Vector_t<String_t>& output)
        {
            if (item && item->Is_Valid()) {
                Vector_t<Actor_Base_t*> bases = item->Actor_Bases();
                for (Index_t idx = 0, end = bases.size(); idx < end; idx += 1) {
                    Selectable_Templates_t<Base_t, Actor_Base_t*>::Select(bases[idx], output);
                }
            }
        }
    };

    template <typename Base_t>
    class Selectable_Templates_t<Base_t, Actor_t*> : public Selectable_Templates_i<Base_t, Actor_t*>
    {
    public:
        using Item_t = Actor_t*;

    public:
        Selectable_Templates_t() :
            Selectable_Templates_i<Base_t, Item_t>(&Select)
        {
        }

        static void Select(Item_t item, Vector_t<String_t>& output)
        {
            if (item && item->Is_Valid()) {
                Selectable_Templates_t<Base_t, Actor_Base_t*>::Select(item->Actor_Base(), output);
            }
        }
    };

    template <typename Base_t>
    class Selectable_Templates_t<Base_t, Loaded_Actor_t> : public Selectable_Templates_i<Base_t, Loaded_Actor_t>
    {
    public:
        using Item_t = Loaded_Actor_t;

    public:
        Selectable_Templates_t() :
            Selectable_Templates_i<Base_t, Item_t>(&Select)
        {
        }

        static void Select(Item_t item, Vector_t<String_t>& output)
        {
            if (item.Is_Valid()) {
                Selectable_Templates_t<Base_t, Actor_Base_t*>::Select(item.actor->Actor_Base(), output);
            }
        }
    };

    template <typename Base_t, typename Item_t>
    class Selectable_References_i : public Selectable_Data_t<Base_t, Item_t>
    {
    public:
        Selectable_References_i(Select_f select_f) :
            Selectable_Data_t<Base_t, Item_t>(&Filter_t::Reference_Argument, &Filter_t::Reference_Argument, select_f)
        {
        }
    };

    template <typename Base_t, typename Item_t>
    class Selectable_References_t : public Selectable_References_i<Base_t, Item_t>
    {
    };

    template <typename Base_t>
    class Selectable_References_t<Base_t, Actor_t*> : public Selectable_References_i<Base_t, Actor_t*>
    {
    public:
        using Item_t = Actor_t*;

    public:
        Selectable_References_t() :
            Selectable_References_i<Base_t, Item_t>(&Select)
        {
        }

        static void Select(Item_t item, Vector_t<String_t>& output)
        {
            if (item && item->Is_Valid()) {
                output.push_back(item->Any_Name());
            }
        }
    };

    template <typename Base_t>
    class Selectable_References_t<Base_t, Loaded_Actor_t> : public Selectable_References_i<Base_t, Loaded_Actor_t>
    {
    public:
        using Item_t = Loaded_Actor_t;

    public:
        Selectable_References_t() :
            Selectable_References_i<Base_t, Item_t>(&Select)
        {
        }

        static void Select(Item_t item, Vector_t<String_t>& output)
        {
            if (item.Is_Valid()) {
                output.push_back(item.actor->Any_Name());
            }
        }
    };

    template <typename Base_t, typename Item_t>
    class Selectable_Locations_i : public Selectable_Data_t<Base_t, Item_t>
    {
    public:
        Selectable_Locations_i(Select_f select_f) :
            Selectable_Data_t<Base_t, Item_t>(&Filter_t::Location_Argument, &Filter_t::Location_Argument, select_f)
        {
        }
    };

    template <typename Base_t, typename Item_t>
    class Selectable_Locations_t : public Selectable_Locations_i<Base_t, Item_t>
    {
    };

    template <typename Base_t>
    class Selectable_Locations_t<Base_t, Cell_t*> : public Selectable_Locations_i<Base_t, Cell_t*>
    {
    public:
        using Item_t = Cell_t*;

    public:
        Selectable_Locations_t() :
            Selectable_Locations_i<Base_t, Item_t>(&Select)
        {
        }

        static void Select(Item_t item, Vector_t<String_t>& output)
        {
            if (item && item->Is_Valid()) {
                item->Location_Names(output);
            }
        }
    };

    template <typename Base_t>
    class Selectable_Locations_t<Base_t, Actor_t*> : public Selectable_Locations_i<Base_t, Actor_t*>
    {
    public:
        using Item_t = Actor_t*;

    public:
        Selectable_Locations_t() :
            Selectable_Locations_i<Base_t, Item_t>(&Select)
        {
        }

        static void Select(Item_t item, Vector_t<String_t>& output)
        {
            if (item && item->Is_Valid()) {
                Selectable_Locations_t<Base_t, Cell_t*>::Select(item->Cell(), output);
            }
        }
    };

    template <typename Base_t>
    class Selectable_Locations_t<Base_t, Loaded_Actor_t> : public Selectable_Locations_i<Base_t, Loaded_Actor_t>
    {
    public:
        using Item_t = Loaded_Actor_t;

    public:
        Selectable_Locations_t() :
            Selectable_Locations_i<Base_t, Item_t>(&Select)
        {
        }

        static void Select(Item_t item, Vector_t<String_t>& output)
        {
            if (item.Is_Valid()) {
                item.cell->Location_Names(output);
            }
        }
    };

    template <typename Base_t, typename Item_t>
    class Selectable_Cells_i : public Selectable_Data_t<Base_t, Item_t>
    {
    public:
        Selectable_Cells_i(Select_f select_f) :
            Selectable_Data_t<Base_t, Item_t>(&Filter_t::Cell_Argument, &Filter_t::Cell_Argument, select_f)
        {
        }
    };

    template <typename Base_t, typename Item_t>
    class Selectable_Cells_t : public Selectable_Cells_i<Base_t, Item_t>
    {
    };

    template <typename Base_t>
    class Selectable_Cells_t<Base_t, Cell_t*> : public Selectable_Cells_i<Base_t, Cell_t*>
    {
    public:
        using Item_t = Cell_t*;

    public:
        Selectable_Cells_t() :
            Selectable_Cells_i<Base_t, Item_t>(&Select)
        {
        }

        static void Select(Item_t item, Vector_t<String_t>& output)
        {
            if (item && item->Is_Valid()) {
                output.push_back(item->Any_Name());
            }
        }
    };

    template <typename Base_t>
    class Selectable_Cells_t<Base_t, Actor_t*> : public Selectable_Cells_i<Base_t, Actor_t*>
    {
    public:
        using Item_t = Actor_t*;

    public:
        Selectable_Cells_t() :
            Selectable_Cells_i<Base_t, Item_t>(&Select)
        {
        }

        static void Select(Item_t item, Vector_t<String_t>& output)
        {
            if (item && item->Is_Valid()) {
                Selectable_Cells_t<Base_t, Cell_t*>::Select(item->Cell(), output);
            }
        }
    };

    template <typename Base_t>
    class Selectable_Cells_t<Base_t, Loaded_Actor_t> : public Selectable_Cells_i<Base_t, Loaded_Actor_t>
    {
    public:
        using Item_t = Loaded_Actor_t;

    public:
        Selectable_Cells_t() :
            Selectable_Cells_i<Base_t, Item_t>(&Select)
        {
        }

        static void Select(Item_t item, Vector_t<String_t>& output)
        {
            if (item.Is_Valid()) {
                output.push_back(item.cell->Any_Name());
            }
        }
    };

    template <typename Base_t, typename Item_t>
    class Selectable_Relations_i : public Selectable_Data_t<Base_t, Item_t>
    {
    public:
        Selectable_Relations_i(Select_Relation_f select_f, Actor_Base_t* base_to_compare) :
            Selectable_Data_t<Base_t, Item_t>(&Filter_t::Relation_Argument, &Filter_t::Relation_Argument, select_f, base_to_compare)
        {
        }
    };

    template <typename Base_t, typename Item_t>
    class Selectable_Relations_t : public Selectable_Relations_i<Base_t, Item_t>
    {
    };

    template <typename Base_t>
    class Selectable_Relations_t<Base_t, Actor_Base_t*> : public Selectable_Relations_i<Base_t, Actor_Base_t*>
    {
    public:
        using Item_t = Actor_Base_t*;

    public:
        Selectable_Relations_t(Actor_Base_t* base_to_compare) :
            Selectable_Relations_i<Base_t, Item_t>(&Select, base_to_compare)
        {
        }

        static void Select(Item_t item, Actor_Base_t* base_to_compare, Vector_t<String_t>& output)
        {
            Relation_e relation = Relation_e::Between(item, base_to_compare);
            if (relation != Relation_e::NONE) {
                output.push_back(std::string(" ") + Relation_e::To_String(relation) + " ");
            }
        }
    };

    template <typename Base_t>
    class Selectable_Relations_t<Base_t, Leveled_Actor_Base_t*> : public Selectable_Relations_i<Base_t, Leveled_Actor_Base_t*>
    {
    public:
        using Item_t = Leveled_Actor_Base_t*;

    public:
        Selectable_Relations_t(Actor_Base_t* base_to_compare) :
            Selectable_Relations_i<Base_t, Item_t>(&Select, base_to_compare)
        {
        }

        static void Select(Item_t item, Actor_Base_t* base_to_compare, Vector_t<String_t>& output)
        {
            Vector_t<Actor_Base_t*> actor_bases = item->Actor_Bases();
            for (Index_t idx = 0, end = actor_bases.size(); idx < end; idx += 1) {
                Actor_Base_t* actor_base = actor_bases[idx];
                if (actor_base) {
                    Relation_e relation = Relation_e::Between(actor_base, base_to_compare);
                    if (relation != Relation_e::NONE) {
                        String_t relation_name = std::string(" ") + Relation_e::To_String(relation) + " ";
                        if (!output.Has(relation_name)) {
                            output.push_back(relation_name);
                        }
                    }
                }
            }
        }
    };

    template <typename Base_t>
    class Selectable_Relations_t<Base_t, Actor_t*> : public Selectable_Relations_i<Base_t, Actor_t*>
    {
    public:
        using Item_t = Actor_t*;

    public:
        Selectable_Relations_t(Actor_Base_t* base_to_compare) :
            Selectable_Relations_i<Base_t, Item_t>(&Select, base_to_compare)
        {
        }

        static void Select(Item_t item, Actor_Base_t* base_to_compare, Vector_t<String_t>& output)
        {
            if (item && item->Is_Valid()) {
                Selectable_Relations_t<Base_t, Actor_Base_t*>::Select(item->Actor_Base(), base_to_compare, output);
            }
        }
    };

    template <typename Base_t>
    class Selectable_Relations_t<Base_t, Loaded_Actor_t> : public Selectable_Relations_i<Base_t, Loaded_Actor_t>
    {
    public:
        using Item_t = Loaded_Actor_t;

    public:
        Selectable_Relations_t(Actor_Base_t* base_to_compare) :
            Selectable_Relations_i<Base_t, Item_t>(&Select, base_to_compare)
        {
        }

        static void Select(Item_t item, Actor_Base_t* base_to_compare, Vector_t<String_t>& output)
        {
            if (item.Is_Valid()) {
                Selectable_Relations_t<Base_t, Actor_Base_t*>::Select(item.actor->Actor_Base(), base_to_compare, output);
            }
        }
    };

}}