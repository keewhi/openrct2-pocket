#include "DesignAvailability.h"
#include "SavedParkPresentation.h"
#include "ShopAppearance.h"
// SPDX-License-Identifier: GPL-3.0-only
#include "Alerts.h"
#include "AreaTools.h"
#include "ChoiceCards.h"
#include "CompactTools.h"
#include "Currency.h"
#include "CustomTracks.h"
#include "Dashboard.h"
#include "DesignAuthoring.h"
#include "Designs.h"
#include "Explorer.h"
#include "FeelSound.h"
#include "FileBrowsing.h"
#include "FinanceInformation.h"
#include "Finances.h"
#include "FlatRides.h"
#include "GuestFollow.h"
#include "GuestManagement.h"
#include "Guide.h"
#include "Hud.h"
#include "ListFocus.h"
#include "ManagementNews.h"
#include "MapSubjects.h"
#include "MarketingManagement.h"
#include "Mazes.h"
#include "MenuThemes.h"
#include "NamedSave.h"
#include "NativeTrackThumbnail.h"
#include "Panel.h"
#include "ParkInformation.h"
#include "ParkManagementTabs.h"
#include "ParkMap.h"
#include "ParkOverview.h"
#include "PathSurfacePicker.h"
#include "Paths.h"
#include "PeopleNavigation.h"
#include "QuickWheel.h"
#include "ResearchCatalog.h"
#include "ResearchManagement.h"
#include "Resume.h"
#include "RideAccess.h"
#include "RideFollow.h"
#include "RideManagement.h"
#include "RideNavigation.h"
#include "RideTabs.h"
#include "RightsTools.h"
#include "SaveSlots.h"
#include "SceneryClear.h"
#include "SceneryTools.h"
#include "ShopManagement.h"
#include "Shops.h"
#include "StaffManagement.h"
#include "StaffPatrol.h"
#include "TerrainMaterials.h"
#include "TextEntry.h"
#include "ToolSettingsPanel.h"
#include "TrackEditing.h"
#include "TreePicker.h"
#include "Trees.h"

#include <cctype>
#include <filesystem>
#include <iomanip>
#include <map>
#include <openrct2-ui/UiContext.h>
#include <openrct2-ui/input/ShortcutIds.h>
#include <openrct2-ui/input/ShortcutManager.h>
#include <openrct2-ui/interface/FileBrowser.h>
#include <openrct2-ui/interface/Widget.h>
#include <openrct2-ui/windows/Windows.h>
#include <openrct2/Context.h>
#include <openrct2/Game.h>
#include <openrct2/GameState.h>
#include <openrct2/OpenRCT2.h>
#include <openrct2/ParkImporter.h>
#include <openrct2/PlatformEnvironment.h>
#include <openrct2/actions/GameActionRunner.h>
#include <openrct2/actions/general/GameSetSpeedAction.h>
#include <openrct2/actions/park/ParkSetResearchFundingAction.h>
#include <openrct2/actions/peep/StaffHireNewAction.h>
#include <openrct2/actions/ride/RideSetStatusAction.h>
#include <openrct2/config/Config.h>
#include <openrct2/core/FileStream.h>
#include <openrct2/entity/Guest.h>
#include <openrct2/entity/Peep.h>
#include <openrct2/interface/Viewport.h>
#include <openrct2/interface/WindowTypes.h>
#include <openrct2/localisation/StringIds.h>
#include <openrct2/object/ObjectManager.h>
#include <openrct2/object/PeepAnimationsObject.h>
#include <openrct2/object/RideObject.h>
#include <openrct2/park/ParkPreview.h>
#include <openrct2/ride/Ride.h>
#include <openrct2/ride/RideData.h>
#include <openrct2/ride/TrackDesignRepository.h>
#include <openrct2/scenario/Scenario.h>
#include <openrct2/ui/WindowManager.h>
#include <openrct2/world/MapSelection.h>
#include <sstream>

namespace OpenRCT2::Ui::Windows
{
    using namespace Drawing;
    namespace fs = std::filesystem;
    static constexpr ScreenSize kSize{ 244, 278 };
    static constexpr auto kWidgets = makeWidgets(makeWindowShim(STR_SHORTCUT_PAUSE, kSize));
    enum class Page
    {
        park,
        controls,
        primer,
        settings,
        build,
        paths,
        pathOptions,
        pathSurfacePicker,
        terrainMaterialPicker,
        pathUndoConfirm,
        treePicker,
        sceneryCategory,
        sceneryPicker,
        sceneryOptions,
        sceneryClearOptions,
        sceneryClearConfirm,
        sceneryGroups,
        sceneryStyle,
        sceneryStyleEditor,
        sceneryStyleConfirm,
        areaOptions,
        terrainPaintConfirm,
        rightsOptions,
        rightsConfirm,
        sceneryName,
        scenerySearch,
        sceneryConfirm,
        shopPicker,
        ridePicker,
        designPicker,
        customChoice,
        trackPiecePicker,
        designConfirm,
        designOptions,
        dashboard,
        overview,
        overviewConfirm,
        parkMap,
        parkNews,
        parkNewsDetail,
        parkAwards,
        parkHistory,
        mapSubjects,
        mapSubject,
        finances,
        financeLedger,
        financeHistory,
        financeEditor,
        financeConfirm,
        staff,
        staffInspect,
        staffStats,
        staffOrders,
        staffHire,
        staffDismiss,
        staffRoles,
        staffCostumes,
        staffName,
        staffNameConfirm,
        staffSummary,
        staffUniform,
        staffPatrolOptions,
        staffPatrolClear,
        guests,
        guestRoster,
        guestInspect,
        guestNeeds,
        guestThoughts,
        guestSpending,
        guestInventory,
        guestText,
        guestGroups,
        guestName,
        guestFilterName,
        guestNameConfirm,
        guestTracking,
        guestPreferences,
        guestRideHistory,
        peepOptions,
        peepCancel,
        ridesRoster,
        ridesInspect,
        trackBlocks,
        trackBlock,
        trackContext,
        trackRemoveConfirm,
        trackReplaceConfirm,
        trackSpeedConfirm,
        ridesAccess,
        accessManage,
        accessPoint,
        accessReview,
        ridesStats,
        ridesCustomers,
        ridesStations,
        ridesEarnings,
        ridesGraphs,
        ridesOperations,
        ridesMaintenance,
        ridesVehicles,
        ridesDispatch,
        ridesMusic,
        ridesAppearance,
        ridesName,
        designName,
        designScenery,
        designExport,
        ridesPrice,
        ridesConfirm,
        research,
        researchCatalog,
        researchItem,
        researchEditor,
        researchConfirm,
        marketing,
        marketingActive,
        marketingTypes,
        marketingEditor,
        marketingTargets,
        marketingConfirm,
        ride,
        shop,
        shopAppearance,
        shopAppearanceEditor,
        shopAppearanceReview,
        shopDemolish,
        save,
        load,
        parkInspect,
        designFiles,
        designInspect,
        designVehicles,
        designWarnings,
        parkFiles,
        saveFiles,
        saveName,
        saveReview,
        overwrite,
        discard,
        exit,
        quitChoice
    };

    Handheld::ManagementPresentation HandheldDashboardPresentation(size_t selected)
    {
        Handheld::ManagementPresentation model;
        model.breadcrumb = "Park management";
        model.title = getGameState().park.name;
        model.selected = selected;
        for (const auto& section : Handheld::Dashboard::kCategories)
            model.rows.emplace_back(section.name);
        return model;
    }

    // The last Build card confirmed, kept beyond the menu window: Build reopens on it and the
    // quick wheel's "Again" repeats it.
    static size_t sLastBuildCard = 8;
    static std::string sRideReturnNotice; // Set by a tool just before it hands back to the ride.
    static std::string sLastBuildLabel;
    static uint32_t sLastBuildSprite{};
    static fs::path SavesFolder()
    {
        return fs::u8path(GetContext()->GetPlatformEnvironment().GetDirectoryPath(DirBase::user, DirId::saves));
    }
    // A park's name as a folder name: characters that FAT (the SD card) refuses become spaces.
    static std::string SlotFolderName(const std::string& park)
    {
        std::string name;
        for (const char c : park)
            name += (static_cast<unsigned char>(c) < 32 || std::string_view("<>:\"/\\|?*").find(c) != std::string_view::npos)
                ? ' '
                : c;
        while (!name.empty() && (name.back() == ' ' || name.back() == '.'))
            name.pop_back();
        while (!name.empty() && name.front() == ' ')
            name.erase(name.begin());
        if (name.size() > 60)
            name.resize(60);
        return name.empty() ? "Park" : name;
    }
    // Quick slots belong to the open park: saves/Pocket/<park>/Slot N.park, three per park.
    static fs::path SlotPath(size_t slot)
    {
        return SavesFolder() / "Pocket" / fs::u8path(SlotFolderName(getGameState().park.name))
            / ("Slot " + std::to_string(slot + 1) + ".park");
    }
    // Earlier builds kept three slots shared by every park (saves/Pocket-N.park). A shared slot
    // holding this park moves into the park's own folder when its slots are first shown; slots
    // holding other parks stay where they are until those parks are open.
    static void AdoptSharedSlots()
    {
        std::error_code error;
        for (size_t slot = 0; slot < 3; ++slot)
        {
            const auto shared = SavesFolder() / ("Pocket-" + std::to_string(slot + 1) + ".park");
            const auto own = SlotPath(slot);
            if (!fs::exists(shared, error) || fs::exists(own, error))
                continue;
            const auto preview = Handheld::SavedParkPresentation::Read(shared);
            if (!preview || preview->parkName != getGameState().park.name)
                continue;
            fs::create_directories(own.parent_path(), error);
            fs::rename(shared, own, error);
        }
    }

    class HandheldMenuWindow;
    static HandheldMenuWindow* OpenPanel();

    class HandheldMenuWindow final : public Window
    {
        Handheld::ListFocus _focus;
        Page _page = Page::park;
        std::map<Page, size_t> _pageRows;
        std::array<std::array<size_t, 5>, 2> _toolGroupRows{};
        Handheld::ObjectPicker _picker;
        bool _pausedHere{};
        size_t _buildFocus{ sLastBuildCard }, _pathModeFocus{}; // Build opens on the card used last (Rides at first).
        std::array<size_t, 2> _pathOptionsFocus{};
        bool _shopPickerFromBuild{};
        bool _ridePickerFromBuild{};
        bool _designPickerFromBuild{};
        ObjectEntryIndex _customObject = kObjectEntryIndexNull;
        ride_type_t _customType = kRideTypeNull;
        bool _exitAfterSave{};
        bool _quitStandalone{}, _quitSaving{};
        Page _quitCaller = Page::park;
        size_t _quitFocus{};
        size_t _slot{};
        Handheld::FileBrowsing::State _files;
        std::optional<Handheld::FileBrowsing::Identity> _loadFile;
        bool _fileLoad{};
        size_t _fileFocus{}, _fileDetail{}, _designDetail{};
        std::optional<ObjectEntryDescriptor> _designChoice;
        std::vector<ObjectEntryDescriptor> _designVehicles;
        std::vector<int32_t> _designVehicleContext;
        bool _designVehicleFromFile{};
        std::optional<Handheld::PickerEntry> _designWorkspacePreview;
        money64 _designQuote = kMoney64Undefined;
        std::string _designReadiness;
        void refreshDesignWorkspace()
        {
            const auto* draft = Handheld::Designs::Draft();
            _designWorkspacePreview = draft ? Handheld::Designs::FileEntry(
                                                  Handheld::Designs::Selection(), draft->trackAndVehicle.vehicleObject, 260,
                                                  144, Handheld::Designs::Orientation())
                                            : std::nullopt;
            const bool release = _pausedHere && (gGamePaused & GAME_PAUSED_NORMAL);
            if (release)
                PauseToggle();
            const auto query = Handheld::Designs::Query(Handheld::CurrentTerrainTile());
            if (release)
                PauseToggle();
            _designQuote = query.error == GameActions::Status::ok || query.error == GameActions::Status::insufficientFunds
                ? query.cost
                : kMoney64Undefined;
            _designReadiness = query.error == GameActions::Status::ok ? "Ready to place"
                : query.getErrorMessage().empty()                     ? query.getErrorTitle()
                                                                      : query.getErrorMessage();
        }
        bool fileDesignEligible() const
        {
            if (!_designMetadata)
                return false;
            auto td = *_designMetadata;
            if (_designChoice)
                td.trackAndVehicle.vehicleObject = *_designChoice;
            return Handheld::Designs::Eligible(td);
        }
        Handheld::ObjectPicker designVehiclePicker() const
        {
            std::vector<Handheld::PickerEntry> entries;
            auto& manager = GetContext()->GetObjectManager();
            for (const auto& d : _designVehicles)
            {
                const auto* object = manager.GetLoadedObject(d);
                if (object)
                    entries.push_back(
                        { { ObjectType::ride, manager.GetLoadedObjectEntryIndex(object), d.ToString() },
                          object->GetName(),
                          ZoomLevel{ 1 } });
            }
            std::optional<Handheld::PickerIdentity> selected;
            if (_focus.index() < entries.size())
                selected = entries[_focus.index()].identity;
            Handheld::ObjectPicker picker;
            picker.setEntries(std::move(entries), selected);
            return picker;
        }
        void openDesignVehicles(bool file)
        {
            _designVehicleFromFile = file;
            _designVehicles.clear();
            const auto* td = file ? _designMetadata.get() : Handheld::Designs::Draft();
            if (td)
            {
                _designVehicleContext = TrackDesignPlacementContext::Capture(*td);
                for (auto i : Handheld::DesignAvailability::Vehicles(*td))
                    if (auto* o = GetContext()->GetObjectManager().GetLoadedObject<RideObject>(i))
                        _designVehicles.push_back(o->GetDescriptor());
            }
            _designDetail = 0;
            page(Page::designVehicles);
        }
        std::optional<ParkPreview> _parkFilePreview;
        std::array<std::optional<ParkPreview>, 3> _slotPreviews;
        bool _designDetailsFromFile{};
        std::string _parkPreviewStatus;
        std::optional<Handheld::FileBrowsing::Identity> _designFile;
        std::unique_ptr<TrackDesign> _designMetadata;
        std::optional<Handheld::PickerEntry> _designFilePreview;
        std::string _designFileStatus;
        bool _namedSave{};
        std::optional<Handheld::NamedSave::Draft> _saveDraft;
        std::string fileDetailPath() const
        {
            const size_t offset = (_page == Page::saveFiles ? 1 : 0) + Handheld::FileBrowsing::kFirst;
            return _focus.index() >= offset && _focus.index() - offset < _files.entries.size()
                ? _files.entries[_focus.index() - offset].path.u8string()
                : _files.directory.u8string();
        }
        void openSaveFiles()
        {
            _namedSave = true;
            _saveDraft.reset();
            _fileFocus = 0;
            _fileDetail = 0;
            _files.Open(fs::u8path(Ui::FileBrowser::GetInitialDirectoryByType(LoadSaveType::park)), "*.park");
            page(Page::saveFiles);
        }
        void editSaveName(std::string name)
        {
            _saveDraft.reset();
            _nameEntry = { name };
            if (!Handheld::NamedSave::SafeName(name))
                _nameEntry.text = "My park";
            page(Page::saveName);
        }
        void openParkFiles()
        {
            _fileLoad = true;
            _fileFocus = 0;
            _fileDetail = 0;
            _files.Open(
                fs::u8path(Ui::FileBrowser::GetInitialDirectoryByType(LoadSaveType::park)),
                Ui::FileBrowser::GetFilterPatternByType(LoadSaveType::park, false));
            page(Page::parkFiles);
        }

        Handheld::Dashboard::State _dashboard;
        struct SectionResume
        {
            std::optional<Page> page;
            size_t row{};
        };
        std::array<SectionResume, Handheld::Dashboard::kCategories.size()> _sectionResume;
        std::string _status;
        RideId _rideId = RideId::GetNull();
        std::optional<Handheld::ShopManagement::Target> _shop;
        std::optional<Handheld::Finances::Draft> _financeDraft;
        Handheld::TabFocus<4> _financeTabs;
        Handheld::TabFocus<2> _researchTabs, _marketingTabs;
        Handheld::TabFocus<3> _parkTabs;
        Page _financeReturnPage = Page::finances;
        bool _ledgerInitialized{}, _civicBusy{};
        std::shared_ptr<int> _civicLifetime = std::make_shared<int>(0);
        Handheld::ManagementPresentation financeWorkspace() const
        {
            return Handheld::ParkManagementTabs::Finance(
                _financeTabs.tab, _focus.index(), _ledger, _financeGraph, _financeSample, _status);
        }
        void financeTab(size_t tab)
        {
            _financeTabs.save(_focus.index());
            _financeTabs.tab = tab;
            if (tab == 1 && !_ledgerInitialized)
            {
                _ledger.Begin();
                _ledgerInitialized = true;
            }
            _focus = {};
            _focus.move(static_cast<int>(std::min(_financeTabs.rows[tab], rows().size() - 1)), rows().size());
            _status.clear();
            invalidate();
        }
        void researchTab(size_t tab)
        {
            if (tab != _researchTabs.tab && guardLeave([this, tab] { researchTab(tab); }))
                return;
            _researchTabs.save(_focus.index());
            _researchTabs.tab = tab;
            if (tab == 1 && !_researchDraft)
                _researchDraft = Handheld::ResearchManagement::Begin();
            if (tab != 1)
                _researchDraft.reset();
            _focus = {};
            _focus.move(static_cast<int>(std::min(_researchTabs.rows[tab], rows().size() - 1)), rows().size());
            _status.clear();
            invalidate();
        }
        void marketingTab(size_t tab)
        {
            _marketingTabs.save(_focus.index());
            _marketingTabs.tab = tab;
            _marketingActiveTypes = Handheld::MarketingManagement::ActiveTypes();
            _focus = {};
            _focus.move(static_cast<int>(std::min(_marketingTabs.rows[tab], rows().size() - 1)), rows().size());
            _status.clear();
            invalidate();
        }
        std::optional<Handheld::ParkOverview::Draft> _overviewDraft;
        Handheld::ParkMap::State _parkMap;
        std::vector<Handheld::ParkInformation::NewsTarget> _parkNews;
        // The inbox: what needs attention now, listed above the news. A snapshot taken when it
        // opens, so the rows do not shift under the cursor as alerts come and go.
        std::vector<Handheld::Alerts::Alert> _parkAlerts;
        std::optional<Handheld::ParkInformation::NewsTarget> _parkNewsTarget;
        size_t _parkNewsPage{}, _parkNewsFocus{};
        bool _newsFromPark{}, _newsFromDashboard{};
        bool _objectivesFromPark{};
        bool _feelBack{};
        size_t _rideCategory{}; // Ride picker filter: index into kRideCategories.
        static constexpr std::array<std::pair<const char*, RideCategory>, 6> kRideCategories{ {
            { "All rides", RideCategory::none },
            { "Roller coasters", RideCategory::rollerCoaster },
            { "Thrill rides", RideCategory::thrill },
            { "Gentle rides", RideCategory::gentle },
            { "Water rides", RideCategory::water },
            { "Transport", RideCategory::transport },
        } };
        static RideCategory EntryCategory(const Handheld::PickerEntry& entry)
        {
            ride_type_t type = kRideTypeNull;
            const auto& key = entry.identity.key;
            if (key.starts_with("custom-type:"))
                type = static_cast<ride_type_t>(std::stoi(key.substr(12)));
            else if (key.starts_with("custom:"))
                type = RIDE_TYPE_WOODEN_ROLLER_COASTER;
            else if (const auto* object = GetContext()->GetObjectManager().GetLoadedObject<RideObject>(entry.identity.index))
                for (auto candidate : object->GetEntry().ride_type)
                    if (candidate != kRideTypeNull)
                    {
                        type = candidate;
                        break;
                    }
            return type == kRideTypeNull ? RideCategory::none : GetRideTypeDescriptor(type).Category;
        }
        // Filters by the selected category, adds the game's ride description, and gives each
        // grid cell the part of the name that tells entries apart (the vehicle, when several
        // entries share a ride type).
        std::vector<Handheld::PickerEntry> rideCatalog() const
        {
            auto all = Handheld::FlatRides::Entries();
            auto mazes = Handheld::Mazes::Entries();
            all.insert(all.end(), mazes.begin(), mazes.end());
            auto custom = Handheld::CustomTracks::RideEntries();
            all.insert(all.end(), custom.begin(), custom.end());
            const auto wanted = kRideCategories[_rideCategory % kRideCategories.size()].second;
            std::vector<Handheld::PickerEntry> result;
            std::map<std::string, int> typeCounts;
            for (auto& entry : all)
            {
                if (wanted != RideCategory::none && EntryCategory(entry) != wanted)
                    continue;
                if (const auto slash = entry.name.find(" / "); slash != std::string::npos)
                    ++typeCounts[entry.name.substr(0, slash)];
                if (const auto* object = GetContext()->GetObjectManager().GetLoadedObject<RideObject>(entry.identity.index))
                    entry.detail = object->GetDescription();
                result.push_back(std::move(entry));
            }
            for (auto& entry : result)
                if (const auto slash = entry.name.find(" / "); slash != std::string::npos)
                {
                    const auto type = entry.name.substr(0, slash);
                    entry.label = typeCounts[type] > 1 ? entry.name.substr(slash + 3) : type;
                }
            return result;
        }
        Handheld::Dashboard::State _newsDashboard;
        bool _parkGuestHistory{};
        bool _managementNewsReturn{};
        std::vector<Handheld::MapSubjects::Target> _mapSubjects;
        std::optional<Handheld::MapSubjects::Target> _mapTarget;
        size_t _mapSubjectsFocus{};
        bool _mapInspection{};
        bool _trackTechnical{}, _designTechnical{};
        Page _mapInspectionReturn = Page::mapSubject;
        void inspectMapSubject(const Handheld::MapSubjects::Target& target, Page parent)
        {
            if (!Handheld::MapSubjects::Current(target))
            {
                _status = "Subject departed or changed";
                invalidate();
                return;
            }
            std::visit(
                [&](const auto& person) {
                    using T = std::decay_t<decltype(person)>;
                    if constexpr (std::is_same_v<T, Handheld::GuestManagement::Target>)
                        openGuest(person);
                    else if constexpr (std::is_same_v<T, Handheld::StaffManagement::Target>)
                    {
                        _dashboard.section = Handheld::Dashboard::Section::staff;
                        openStaff(person);
                    }
                    else
                    {
                        _ridesTarget = person;
                        _dashboard.section = Handheld::Dashboard::Section::rides;
                        _dashboard.focus = Handheld::Dashboard::Focus::content;
                        if (person.shop)
                        {
                            _shop = Handheld::ShopManagement::Capture(person.id);
                            page(Page::shop);
                        }
                        else
                            page(Page::ridesInspect);
                    }
                },
                target);
            _mapInspection = true;
            _mapInspectionReturn = parent;
        }
        Handheld::FinanceInformation::Ledger _ledger;
        size_t _financeGraph{}, _financeSample{};
        uint8_t _researchFilter{};
        std::vector<Handheld::ResearchCatalog::Target> _researchCatalog;
        std::optional<Handheld::ResearchCatalog::Target> _researchItem;
        size_t _researchCatalogFocus{};
        // Sheets laid out for the Brick's 512x384 canvas keep their width, and stop above the
        // footer on a shorter canvas (640x360 on 16:9 handhelds).
        static int SheetHeight()
        {
            return std::min(348, ContextGetHeight() - 36);
        }
        bool parkInformation() const
        {
            return _page == Page::researchCatalog || _page == Page::researchItem || _page == Page::financeHistory
                || _page == Page::financeLedger || _page == Page::mapSubjects || _page == Page::mapSubject
                || _page == Page::parkNews || _page == Page::parkNewsDetail || _page == Page::parkAwards
                || _page == Page::parkHistory;
        }
        std::vector<size_t> newsActions() const
        {
            std::vector<size_t> actions;
            if (!_parkNewsTarget)
                return actions;
            const auto& target = *_parkNewsTarget;
            if (Handheld::ParkInformation::SubjectCurrent(target)
                || Handheld::ManagementNews::Available(target) != Handheld::ManagementNews::Link::none)
                actions.push_back(2);
            if (Handheld::ParkInformation::Location(target))
                actions.push_back(1);
            actions.push_back(3);
            if (Handheld::ParkInformation::IsCurrentMessage(target))
                actions.push_back(4);
            if (Handheld::GuestManagement::TextLines(Handheld::ParkInformation::PlainText(target.item.text)).size() > 6)
                actions.push_back(0);
            return actions;
        }
        Handheld::ManagementPresentation parkInformationPresentation() const
        {
            using namespace Handheld::ParkInformation;
            if (_page == Page::researchCatalog)
                return Handheld::ResearchCatalog::Presentation(_researchCatalog, _researchFilter, _focus.index());
            if (_page == Page::researchItem && _researchItem)
                return Handheld::ResearchCatalog::Detail(*_researchItem, _focus.index());
            if (_page == Page::financeHistory)
                return Handheld::FinanceInformation::History(_financeGraph, _financeSample, _focus.index());
            if (_page == Page::financeLedger)
                return Handheld::FinanceInformation::LedgerPresentation(_ledger, _focus.index());
            if (_page == Page::mapSubjects)
                return Handheld::MapSubjects::Presentation(
                    _mapSubjects, _parkMap.mode, _parkMap.filter, _focus.index(), _parkMap.guestFilter);
            if (_page == Page::mapSubject && _mapTarget)
                return Handheld::MapSubjects::Detail(*_mapTarget, _focus.index(), _status);
            if (_page == Page::parkNews)
            {
                auto model = NewsPresentation(_parkNews, _focus.index());
                if (!_parkAlerts.empty())
                {
                    model.title = "Inbox";
                    model.breadcrumb = "Park / Inbox";
                    std::vector<std::string> alerts;
                    for (const auto& alert : _parkAlerts)
                        alerts.push_back(alert.title);
                    if (_parkNews.empty())
                        model.rows.clear();
                    model.rows.insert(model.rows.begin(), alerts.begin(), alerts.end());
                    const auto focused = _focus.index();
                    model.status = focused < _parkAlerts.size() ? _parkAlerts[focused].fix + "  |  A: go there"
                                                                : std::to_string(_parkAlerts.size())
                            + (_parkAlerts.size() == 1 ? " thing needs" : " things need") + " attention";
                }
                return model;
            }
            if (_page == Page::parkNewsDetail && _parkNewsTarget)
            {
                auto model = NewsDetail(*_parkNewsTarget, _parkNewsPage, _focus.index(), _status);
                model.rows[2] = Handheld::ManagementNews::Label(*_parkNewsTarget);
                const auto source = model.rows;
                model.rows.clear();
                for (const auto action : newsActions())
                    if (action < source.size())
                        model.rows.push_back(source[action]);
                const auto& target = *_parkNewsTarget;
                std::string subject;
                if (target.ride)
                {
                    if (const auto* ride = Handheld::RideManagement::Resolve(*target.ride))
                        subject = ride->getName();
                }
                else if (target.guest)
                {
                    if (const auto* guest = Handheld::GuestManagement::Resolve(*target.guest))
                        subject = guest->getName();
                }
                else if (target.staff)
                {
                    if (const auto* staff = Handheld::StaffManagement::Resolve(*target.staff))
                        subject = staff->getName();
                }
                if (!subject.empty())
                    model.stats = { { "Subject", subject } };
                if (Handheld::GuestManagement::TextLines(PlainText(target.item.text)).size() <= 6 && _status.empty())
                    model.status.clear();
                return model;
            }
            if (_page == Page::parkAwards)
                return Awards(_focus.index());
            return History(_parkGuestHistory, _focus.index());
        }
        bool _overviewBusy{};
        uint64_t _overviewRequest{};
        std::shared_ptr<int> _overviewLifetime = std::make_shared<int>(0);
        bool overviewManagement() const
        {
            return _page == Page::overview || _page == Page::overviewConfirm;
        }
        Handheld::ManagementPresentation overviewPresentation() const
        {
            return _page == Page::overview ? Handheld::ParkManagementTabs::Park(_parkTabs.tab, _focus.index(), _status)
                                           : Handheld::ParkOverview::Presentation(_focus.index(), _overviewDraft, _status);
        }
        std::optional<Handheld::ResearchManagement::Draft> _researchDraft;
        std::optional<Handheld::MarketingManagement::Draft> _marketingDraft;
        std::vector<int> _marketingTypes, _marketingActiveTypes;
        std::vector<Handheld::MarketingManagement::Target> _marketingTargets;
        int _marketingActiveType = -1;
        bool _marketingBusy{};
        std::shared_ptr<int> _marketingLifetime = std::make_shared<int>(0);
        std::vector<Handheld::GuestManagement::Target> _guestRoster;
        std::optional<Handheld::GuestManagement::Target> _guestTarget;
        size_t _guestRosterFocus = 3, _guestGroupFocus{};
        std::vector<Handheld::RideManagement::Target> _guestRides;
        size_t _guestRideFocus{};
        bool _guestRideInspection{};
        Handheld::PeopleTabs::State _staffTabs, _guestTabs;
        bool _staffFromPark{}, _guestFromPark{};
        Handheld::GuestManagement::GroupMode _guestGroupMode{};
        std::vector<Handheld::GuestManagement::Group> _guestGroups;
        Handheld::GuestManagement::Filter _guestFilter;
        Page _guestRosterParent = Page::guests;
        bool _guestMapReturn{};
        std::optional<Handheld::RideManagement::Target> _guestRosterRideParent;
        Page _guestTextParent = Page::guestThoughts;
        std::string _guestNameBaseline;
        PeepFlags _guestFlagsBaseline{};
        bool _guestTrackingValue{};
        std::string _guestText;
        size_t _guestTextPage{};
        std::vector<Handheld::RideManagement::Target> _ridesRoster;
        std::optional<Handheld::RideManagement::Target> _ridesTarget;
        std::optional<Handheld::RideManagement::Draft> _ridesDraft;
        size_t _ridesRosterFocus{};
        size_t _ridesInspectFocus{};
        bool _rideFromPark{};
        std::map<std::string, Handheld::ListScroll> _scrollViews;
        Handheld::MenuFeel _menuFeel;
        std::string scrollKey() const
        {
            return std::to_string(static_cast<int>(_page)) + ":" + std::to_string(_rideTabs.tab) + ":"
                + std::to_string(_guestTabs.tab) + ":" + std::to_string(_staffTabs.tab) + ":" + std::to_string(_financeTabs.tab)
                + ":" + std::to_string(_researchTabs.tab) + ":" + std::to_string(_parkTabs.tab) + ":"
                + std::to_string(_marketingTabs.tab);
        }
        Handheld::RideTabs::State _rideTabs;
        bool _ridesTabChild{};
        bool _ridesBusy{};
        std::vector<Handheld::RideAccess::ExistingPoint> _accessPoints;
        std::optional<Handheld::RideAccess::ExistingPoint> _accessPoint;
        std::optional<Handheld::RideAccess::EditDraft> _accessDraft;
        size_t _accessFocus{};
        Handheld::TextEntry _nameEntry;
        Page _scenerySearchParent = Page::sceneryOptions;
        size_t _scenerySearchRow{};
        std::optional<Handheld::SceneryTools::Target> _signTarget;
        std::optional<Handheld::SceneryTools::StyleDraft> _signStyle;
        std::optional<Handheld::SceneryClear::Draft> _clearDraft;
        bool _clearReviewResumed{};
        std::optional<Handheld::AreaTools::PaintDraft> _paintDraft;
        bool _paintReviewResumed{};
        bool _pathUndoResumed{};
        std::optional<Handheld::Paths::UndoDraft> _pathUndo;
        std::optional<Handheld::RightsTools::Draft> _rightsDraft;
        bool _rightsReviewResumed{};
        size_t _rideGraphMetric{}, _rideGraphOffset{};
        bool _ridesReviewResumed{};
        uint64_t _ridesRequest{};
        std::shared_ptr<int> _ridesLifetime = std::make_shared<int>(0);
        bool rideVehicleChoice() const
        {
            return _page == Page::ridesPrice && _ridesDraft && _ridesDraft->setting
                && _ridesDraft->setting->vehicle == GameActions::RideSetVehicleType::rideEntry;
        }
        Handheld::ObjectPicker rideVehiclePicker() const
        {
            std::vector<Handheld::PickerEntry> entries;
            if (rideVehicleChoice())
                for (size_t i = 0; i < _ridesDraft->setting->values.size(); ++i)
                    entries.push_back(
                        { { ObjectType::ride, static_cast<ObjectEntryIndex>(_ridesDraft->setting->values[i]) },
                          _ridesDraft->setting->labels[i],
                          ZoomLevel{ 1 } });
            std::optional<Handheld::PickerIdentity> selected;
            if (_focus.index() < entries.size())
                selected = entries[_focus.index()].identity;
            Handheld::ObjectPicker picker;
            picker.setEntries(std::move(entries), selected);
            return picker;
        }
        Page ridesDraftParent() const
        {
            if (_ridesDraft && !_ridesDraft->randomColours.empty())
                return Page::ridesAppearance;
            if (_ridesDraft && _ridesDraft->renaming)
                return Page::ridesName;
            if (_ridesDraft && _ridesDraft->setting
                && _ridesDraft->setting->section == Handheld::RideManagement::View::appearance)
                return Page::ridesAppearance;
            if (_ridesDraft && _ridesDraft->refurbishment)
                return Page::ridesMaintenance;
            if (_ridesDraft && _ridesDraft->setting
                && _ridesDraft->setting->section == Handheld::RideManagement::View::vehicles)
                return Page::ridesVehicles;
            if (_ridesDraft && _ridesDraft->setting
                && _ridesDraft->setting->section == Handheld::RideManagement::View::dispatch)
                return Page::ridesDispatch;
            if (_ridesDraft && _ridesDraft->setting && _ridesDraft->setting->section == Handheld::RideManagement::View::music)
                return Page::ridesMusic;
            if (_ridesDraft && _ridesDraft->setting)
                return _ridesDraft->setting->key == GameActions::RideSetSetting::inspectionInterval ? Page::ridesMaintenance
                                                                                                    : Page::ridesOperations;
            return _ridesTarget && Handheld::RideManagement::Resolve(*_ridesTarget) ? Page::ridesInspect : Page::ridesRoster;
        }
        std::vector<Handheld::TrackEditing::Block> _trackBlocks;
        std::optional<Handheld::TrackEditing::Block> _trackBlock, _trackPrevious;
        std::vector<int32_t> _trackContext;
        money64 _trackCost{};
        uint8_t _trackSpeed{};
        bool trackManagement() const
        {
            return _page == Page::trackBlocks || _page == Page::trackBlock || _page == Page::trackContext
                || _page == Page::trackRemoveConfirm || _page == Page::trackSpeedConfirm || _page == Page::trackReplaceConfirm;
        }
        bool ridesManagement() const
        {
            return _page == Page::accessManage || _page == Page::accessPoint || _page == Page::accessReview || trackManagement()
                || _page == Page::ridesRoster || _page == Page::ridesInspect || _page == Page::ridesGraphs
                || _page == Page::ridesStations || _page == Page::ridesCustomers || _page == Page::ridesEarnings
                || _page == Page::ridesStats || _page == Page::ridesVehicles || _page == Page::ridesName
                || _page == Page::designName || _page == Page::designScenery || _page == Page::designExport
                || _page == Page::ridesAppearance || _page == Page::ridesMusic || _page == Page::ridesDispatch
                || _page == Page::ridesOperations || _page == Page::ridesMaintenance || _page == Page::ridesAccess
                || _page == Page::ridesPrice || _page == Page::ridesConfirm;
        }
        // A guest at a glance: how they look and feel, then the three needs that drive what they do.
        static Handheld::ManagementPresentation::Hero guestHero(const Guest& guest)
        {
            Handheld::ManagementPresentation::Hero hero;
            const auto animations = guest.animationObjectIndex;
            const auto group = guest.animationGroup;
            const auto shirt = guest.tShirtColour, trousers = guest.trousersColour;
            const auto face = GetPeepFaceSpriteLarge(const_cast<Guest*>(&guest));
            hero.picture = [=](RenderTarget& rt) {
                if (const auto* object = GetContext()->GetObjectManager().GetLoadedObject<PeepAnimationsObject>(animations))
                    GfxDrawSprite(rt, ImageId(object->GetPeepAnimation(group).baseImage + 1, shirt, trousers), { 16, 46 });
                GfxDrawSprite(rt, ImageId(face), { 28, 14 });
            };
            const auto percent = [](int value, int maximum) { return std::clamp(value * 100 / std::max(1, maximum), 0, 100); };
            const int happy = percent(guest.happiness, 255);
            const int energy = percent(guest.energy - 32, 128 - 32);
            const int nausea = percent(guest.nausea, 255);
            hero.meters.push_back(
                { "Happiness", std::to_string(happy) + "%", happy,
                  happy >= 60      ? "{POSITIVE}"
                      : happy < 30 ? "{DANGER}"
                                   : "{WHITE}" });
            hero.meters.push_back({ "Energy", std::to_string(energy) + "%", energy, energy < 25 ? "{CAUTION}" : "{WHITE}" });
            hero.meters.push_back({ "Nausea", std::to_string(nausea) + "%", nausea, nausea >= 50 ? "{DANGER}" : "{WHITE}" });
            return hero;
        }
        // The ride's own picture, its state as a switch, and its ratings: what makes a ride a ride.
        static Handheld::ManagementPresentation::Hero rideHero(const Ride& ride)
        {
            Handheld::ManagementPresentation::Hero hero;
            const auto subtype = ride.subtype;
            hero.picture = [subtype](RenderTarget& rt) {
                if (const auto* object = GetContext()->GetObjectManager().GetLoadedObject(ObjectType::ride, subtype))
                {
                    rt.zoom_level = ZoomLevel{ 1 };
                    object->DrawPreview(rt, 128, 120);
                }
            };
            hero.states = { "Closed" };
            if (ride.supportsStatus(RideStatus::testing))
                hero.states.push_back("Testing");
            hero.states.push_back("Open");
            const auto current = ride.status == RideStatus::open ? "Open"
                : ride.status == RideStatus::testing             ? "Testing"
                                                                 : "Closed";
            hero.state = static_cast<size_t>(std::find(hero.states.begin(), hero.states.end(), current) - hero.states.begin());
            auto rating = [](RideRating_t value) {
                char text[16];
                std::snprintf(text, sizeof(text), "%d.%02d", value / 100, value % 100);
                return std::string(text);
            };
            if (!ride.ratings.isNull())
            {
                const auto& r = ride.ratings;
                hero.meters.push_back(
                    { "Excitement", rating(r.excitement), std::min(100, r.excitement / 10),
                      r.excitement >= 500 ? "{POSITIVE}" : "{WHITE}" });
                hero.meters.push_back(
                    { "Intensity", rating(r.intensity), std::min(100, r.intensity / 10),
                      r.intensity >= 1000      ? "{DANGER}"
                          : r.intensity >= 700 ? "{CAUTION}"
                                               : "{WHITE}" });
                hero.meters.push_back(
                    { "Nausea", rating(r.nausea), std::min(100, r.nausea / 10),
                      r.nausea >= 700       ? "{DANGER}"
                          : r.nausea >= 500 ? "{CAUTION}"
                                            : "{WHITE}" });
            }
            else
                hero.meters.push_back({ "Ratings", "Test the ride to rate it", 0, "{MUTED}" });
            return hero;
        }
        Handheld::ManagementPresentation ridesPresentationFor(Page current, size_t selected) const
        {
            using Handheld::RideManagement::View;
            if (trackManagement())
            {
                Handheld::ManagementPresentation model;
                model.breadcrumb = "Dashboard / Rides / Track";
                model.title = current == Page::trackBlocks ? "SELECT TRACK BLOCK" : "TRACK BLOCK";
                model.selected = selected;
                model.primaryAction = "Select";
                model.status = _status;
                if (current == Page::trackBlocks)
                {
                    for (const auto& block : _trackBlocks)
                        model.rows.push_back(Handheld::TrackEditing::Label(block));
                    model.rows.push_back("Back to ride");
                }
                else if (_trackBlock && Handheld::TrackEditing::Valid(*_trackBlock))
                {
                    model.stats.emplace_back("Block", Handheld::TrackEditing::Label(*_trackBlock));
                    model.stats.emplace_back(
                        "Variant",
                        _trackBlock->inverted                                          ? "Inverted"
                            : TrackConstructionSupport::CoveredBase(_trackBlock->type) ? "Covered"
                                                                                       : "Normal");
                    if (TrackConstructionSupport::SeatRotation(_trackBlock->ride.type, _trackBlock->type))
                        model.stats.emplace_back("Seat", std::to_string(_trackBlock->seat));
                    if (trackTypeHasSpeedSetting(_trackBlock->type))
                        model.stats.emplace_back(
                            "Speed", FormatStringID(STR_VELOCITY, static_cast<uint16_t>((_trackSpeed * 9) >> 2)));
                    model.rows = { "Locate block", "Remove block...", "Back to blocks" };
                    if (trackTypeHasSpeedSetting(_trackBlock->type))
                    {
                        model.rows.push_back("Speed -");
                        model.rows.push_back("Speed +");
                        model.rows.push_back("Review speed...");
                    }
                    model.rows.push_back("Continue section...");
                    model.rows.push_back("Build backward...");
                    if (Handheld::TrackEditing::CanReplace(*_trackBlock))
                        model.rows.push_back("Replace block...");
                    model.rows.push_back("Inspect connections / station...");
                    if (current == Page::trackBlock)
                    {
                        const std::vector<std::string> order{ "Continue section...",
                                                              "Replace block...",
                                                              "Remove block...",
                                                              "Build backward...",
                                                              "Speed -",
                                                              "Speed +",
                                                              "Review speed...",
                                                              "Inspect connections / station...",
                                                              "Locate block",
                                                              "Back to blocks" };
                        std::vector<std::string> sorted;
                        for (const auto& label : order)
                            if (std::find(model.rows.begin(), model.rows.end(), label) != model.rows.end())
                                sorted.push_back(label);
                        model.rows = std::move(sorted);
                    }

                    if (current == Page::trackContext)
                    {
                        model.title = "TRACK CONNECTIONS";
                        model.stats.clear();
                        model.rows = _trackTechnical ? Handheld::TrackEditing::Inspection(*_trackBlock, true)
                                                     : std::vector<std::string>{ "Technical details...", "Back to piece" };
                        if (_trackTechnical)
                            model.rowReadOnly = std::vector<bool>(model.rows.size() - 2, true);
                        else
                            for (const auto& line : Handheld::TrackEditing::Inspection(*_trackBlock))
                            {
                                const auto colon = line.find(": ");
                                if (colon != std::string::npos
                                    && (line.starts_with("Ahead:") || line.starts_with("Behind:")
                                        || line.starts_with("Entrance:") || line.starts_with("Exit:")))
                                    model.stats.emplace_back(line.substr(0, colon), line.substr(colon + 2));
                            }
                        model.status = _status;
                    }
                    if (current == Page::trackRemoveConfirm || current == Page::trackSpeedConfirm
                        || current == Page::trackReplaceConfirm)
                    {
                        model.confirmation = true;
                        model.title = current == Page::trackReplaceConfirm ? "REMOVE / REBUILD BLOCK?"
                            : current == Page::trackRemoveConfirm          ? "REMOVE BLOCK?"
                                                                           : "CHANGE TRACK SPEED?";
                        if (current == Page::trackReplaceConfirm)
                        {
                            model.stats.clear();
                            model.bodyLines = { "Remove: " + Handheld::TrackEditing::Label(*_trackBlock),
                                                "Then choose / build at this location.", "Removal leaves a gap; no rollback." };
                            if (_trackBlock->type == TrackElemType::beginStation
                                || _trackBlock->type == TrackElemType::middleStation
                                || _trackBlock->type == TrackElemType::endStation)
                            {
                                model.bodyLines.push_back("Station sections may be renumbered.");
                                model.bodyLines.push_back(Handheld::TrackEditing::AccessRemovalLabel(*_trackBlock));
                                model.bodyLines.push_back("Rebuild deleted entrances/exits explicitly.");
                            }
                        }
                        if (current == Page::trackRemoveConfirm
                            && (_trackBlock->type == TrackElemType::beginStation
                                || _trackBlock->type == TrackElemType::middleStation
                                || _trackBlock->type == TrackElemType::endStation))
                        {
                            model.stats.clear();
                            model.bodyLines = { "Remove: " + Handheld::TrackEditing::Label(*_trackBlock),
                                                "Station sections may be renumbered.",
                                                Handheld::TrackEditing::AccessRemovalLabel(*_trackBlock),
                                                "Rebuild deleted entrances/exits explicitly." };
                        }
                        model.rows = { "Cancel", "Confirm" };
                        model.status = current != Page::trackSpeedConfirm && _trackCost < 0
                            ? "Refund " + Handheld::FormatMoney(-_trackCost)
                            : "Cost " + Handheld::FormatMoney(_trackCost);
                    }
                }
                else
                {
                    model.rows = { "Back to blocks" };
                    model.status = "Block changed; select again";
                }
                return model;
            }
            if (current == Page::designScenery)
            {
                Handheld::ManagementPresentation model;
                model.breadcrumb = "Rides / Save / Scenery";
                model.title = "DESIGN SCENERY";
                model.stats.emplace_back("Selected", std::to_string(Handheld::DesignAuthoring::SceneryCount()));
                model.rows = { "Name / export...", "Select nearby scenery", "No scenery" };
                for (size_t i = 0; i < Handheld::DesignAuthoring::sceneryBaseline.size(); ++i)
                    model.rows.push_back(Handheld::DesignAuthoring::SceneryLabel(i));
                model.selected = selected;
                model.primaryAction = selected >= 3 ? "Toggle" : "Choose";
                model.status = _status.empty() ? "Choose scenery to include in the design" : _status;
                return model;
            }
            if (current == Page::designExport)
            {
                Handheld::ManagementPresentation model;
                model.breadcrumb = "Rides / Save design";
                model.title = Handheld::DesignAuthoring::Overwrite() ? "OVERWRITE DESIGN?" : "EXPORT DESIGN?";
                model.bodyLines.push_back("File: " + Handheld::DesignAuthoring::Filename());
                model.bodyLines.push_back("Scenery: " + std::to_string(Handheld::DesignAuthoring::SceneryCount()));
                model.rows = { "Cancel", "Export design" };
                model.confirmation = true;
                model.selected = selected;
                model.status = _status.empty() ? "Includes tested ride ratings" : _status;
                model.primaryAction = "Choose";
                return model;
            }
            if (current == Page::ridesName || current == Page::designName)
            {
                Handheld::ManagementPresentation model;
                model.breadcrumb = "Name / Keyboard";
                model.title = current == Page::designName ? "Design name" : "Default name";
                model.keyboard = _nameEntry;
                model.keyboardHeading = current == Page::designName ? "Name this design" : "Rename ride";
                model.rows = _nameEntry.Keys();
                model.columns = Handheld::TextEntry::columns;
                model.selected = selected;
                model.status = _status;
                model.primaryAction = "Type";
                return model;
            }
            if (current == Page::accessReview && _accessDraft)
                return Handheld::RideAccess::ReviewPresentation(*_accessDraft, selected, _status);
            if (current == Page::accessManage || current == Page::accessPoint)
            {
                Handheld::ManagementPresentation model;
                model.title = "Entrances / exits";
                model.breadcrumb = "Rides / Access";
                model.selected = selected;
                if (current == Page::accessManage)
                {
                    for (const auto& point : _accessPoints)
                        model.rows.push_back(Handheld::RideAccess::PointLabel(point));
                    model.rows.push_back("Back to ride");
                    model.status = _accessPoints.empty() ? "No supported existing access points"
                                                         : "Select a station entrance or exit";
                }
                else if (_accessPoint)
                {
                    const auto& point = *_accessPoint;
                    model.title = Handheld::RideAccess::PointLabel(point);
                    model.rows = Handheld::RideAccess::Current(point)
                        ? std::vector<std::string>{ "Move access...", "Remove access...", "Back to access list" }
                        : std::vector<std::string>{ "Back to access list" };
                    if (canConnectAccessPoint())
                        model.rows.insert(
                            model.rows.end() - 1, point.exit ? "Connect exit path..." : "Connect entrance queue...");
                    const auto* ride = Handheld::RideManagement::Resolve(point.target);
                    model.status = !Handheld::RideAccess::Current(point) ? "Access changed; select it again"
                        : ride && ride->status == RideStatus::closed     ? "Move keeps source until Apply"
                                                                         : "Close the ride before editing access";
                }
                if (!_status.empty())
                    model.status = _status;
                return model;
            }
            if (current == Page::ridesAccess && _ridesTarget)
                return Handheld::RideAccess::Presentation(*_ridesTarget, selected, _status);
            auto model = Handheld::RideManagement::Presentation(
                current == Page::ridesRoster            ? View::roster
                    : current == Page::ridesInspect     ? View::inspection
                    : current == Page::ridesGraphs      ? View::graphs
                    : current == Page::ridesStations    ? View::stations
                    : current == Page::ridesCustomers   ? View::customers
                    : current == Page::ridesEarnings    ? View::earnings
                    : current == Page::ridesStats       ? View::statistics
                    : current == Page::ridesOperations  ? View::operations
                    : current == Page::ridesMaintenance ? View::maintenance
                    : current == Page::ridesVehicles    ? View::vehicles
                    : current == Page::ridesAppearance  ? View::appearance
                    : current == Page::ridesMusic       ? View::music
                    : current == Page::ridesDispatch    ? View::dispatch
                    : current == Page::ridesPrice       ? View::price
                                                        : View::confirmation,
                selected, _ridesRoster, _ridesTarget, _ridesDraft, _status, _rideGraphMetric, _rideGraphOffset);
            if (current == Page::ridesPrice && rideVehicleChoice())
            {
                model.rows = _ridesDraft->setting->labels;
                model.selected = selected;
                model.stats.clear();
            }
            if (current == Page::ridesInspect)
            {
                const auto stats = Handheld::RideManagement::Presentation(View::statistics, 0, {}, _ridesTarget);
                model = Handheld::RideTabs::Compose(model, stats, _rideTabs, selected);
                if (_ridesTarget && Handheld::RideAccess::Supported(*_ridesTarget))
                    model.contextualAction = "Access";
                if (const auto* ride = _ridesTarget ? Handheld::RideManagement::Resolve(*_ridesTarget) : nullptr)
                {
                    model.headerInk = ride->status == RideStatus::open ? "{POSITIVE}"
                        : ride->status == RideStatus::testing          ? "{CAUTION}"
                                                                       : "{MUTED}";
                    if (_rideTabs.tab == Handheld::RideTabs::overview)
                    {
                        model.hero = rideHero(*ride);
                        if (selected == 0)
                        {
                            model.navigationHint = "Change";
                            model.primaryAction = "Next";
                        }
                        if (!model.settingRows.empty())
                        {
                            const bool priced = Handheld::RideManagement::PricePermitted(*ride);
                            model.settingValues = { priced ? Handheld::FormatMoney(ride->price[0]) : "Not charged" };
                            // A free ride has nothing to adjust, so the footer offers nothing for it.
                            if (selected == model.settingRows.front() && priced)
                            {
                                model.navigationHint = "Price";
                                model.primaryAction = "Editor";
                            }
                        }
                    }
                }
            }
            if (current == Page::ridesConfirm && _ridesDraft && _ridesDraft->demolition)
                model.destructive = true;
            if (current == Page::ridesRoster)
                model.contentSized = true;
            if (current == Page::ridesCustomers && _ridesTarget && Handheld::RideManagement::Resolve(*_ridesTarget))
                model.rows.insert(
                    model.rows.end(), { "Guests riding...", "Guests queuing...", "Guests thinking about ride..." });
            return model;
        }
        std::vector<Handheld::StaffManagement::Target> _staffRoster;
        std::optional<Handheld::StaffManagement::Target> _staffTarget;
        StaffType _hireType = StaffType::handyman;
        uint8_t _ordersBaseline{}, _ordersDraft{};
        ObjectEntryIndex _costumeBaseline = kObjectEntryIndexNull;
        std::optional<Handheld::StaffPatrol::Draft> _patrolDraft;
        Drawing::Colour _staffUniformBaseline{}, _staffUniformDraft{};
        std::string _staffNameBaseline;
        std::optional<StaffType> _staffFilter;
        bool _hireManual{};
        bool _staffBusy{};
        size_t _staffRosterFocus{};
        std::shared_ptr<int> _staffLifetime = std::make_shared<int>(0);
        uint64_t _staffRequest{};
        bool staffManagement() const
        {
            return _page == Page::staff || _page == Page::staffInspect || _page == Page::staffStats
                || _page == Page::staffOrders || _page == Page::staffHire || _page == Page::staffDismiss
                || _page == Page::staffRoles || _page == Page::staffCostumes || _page == Page::staffName
                || _page == Page::staffNameConfirm || _page == Page::staffSummary || _page == Page::staffUniform;
        }
        Handheld::ManagementPresentation staffPresentationFor(Page currentPage, size_t selected) const
        {
            using Handheld::StaffManagement::View;
            if (currentPage == Page::staffUniform && _staffTarget)
                return Handheld::StaffManagement::UniformPresentation(*_staffTarget, _staffUniformDraft, selected);
            if (currentPage == Page::staffName || currentPage == Page::staffNameConfirm)
                return Handheld::StaffManagement::NamePresentation(
                    _nameEntry, selected, currentPage == Page::staffNameConfirm, _status);
            const auto view = currentPage == Page::staffSummary ? View::summary
                : currentPage == Page::staffRoles               ? View::roles
                : currentPage == Page::staffCostumes            ? View::costumes
                : currentPage == Page::staff                    ? View::roster
                : currentPage == Page::staffInspect             ? View::inspection
                : currentPage == Page::staffStats               ? View::statistics
                : currentPage == Page::staffOrders              ? View::orders
                : currentPage == Page::staffHire                ? View::hire
                                                                : View::dismissal;
            auto model = Handheld::StaffManagement::Presentation(
                view, selected, _staffRoster, _staffTarget, _hireType, _ordersDraft, _status);
            if (currentPage == Page::staffInspect && _staffTarget)
            {
                model = Handheld::PeopleTabs::Staff(*_staffTarget, _staffTabs, selected, _ordersDraft);
                if (!_status.empty())
                    model.status = _status;
            }
            if (currentPage == Page::staffRoles)
                model.rows.push_back(_hireManual ? "Placement: Manual" : "Placement: Automatic");
            if (currentPage == Page::staffHire)
            {
                model.stats.back().second = _hireManual ? "Manual" : "Automatic";
                const auto query = Handheld::StaffManagement::Hire(_hireType, false, {}, _hireManual);
                model.status = query.error == GameActions::Status::ok ? _status : Handheld::Finances::Feedback(query);
            }
            if (currentPage == Page::staff)
            {
                model.rows[1] = "Filter: " + (_staffFilter ? Handheld::StaffManagement::RoleName(*_staffFilter) : "All staff");
                if (_staffFilter && _staffRoster.empty() && _status.empty())
                    model.status = "No employees in this role";
            }
            return model;
        }
        Handheld::ManagementPresentation staffPresentation() const
        {
            if (_leaveAction)
                return leavePresentation();
            return staffPresentationFor(_page, _focus.index());
        }
        bool financeManagement() const
        {
            return _page == Page::finances || _page == Page::financeEditor || _page == Page::financeConfirm;
        }
        bool researchManagement() const
        {
            return _page == Page::research || _page == Page::researchEditor || _page == Page::researchConfirm;
        }
        Handheld::ManagementPresentation researchPresentation() const
        {
            if (_leaveAction)
                return leavePresentation();
            using Handheld::ResearchManagement::View;
            if (_page == Page::research)
                return Handheld::ParkManagementTabs::Research(_researchTabs.tab, _focus.index(), _researchDraft, _status);
            if (_page == Page::researchConfirm && _researchDraft)
                return Handheld::ParkManagementTabs::ResearchReview(*_researchDraft, _focus.index(), _status);
            return Handheld::ResearchManagement::Presentation(
                _page == Page::research             ? View::overview
                    : _page == Page::researchEditor ? View::editor
                                                    : View::confirmation,
                _focus.index(), _researchDraft, _status);
        }
        bool marketingManagement() const
        {
            return _page == Page::marketing || _page == Page::marketingActive || _page == Page::marketingTypes
                || _page == Page::marketingEditor || _page == Page::marketingTargets || _page == Page::marketingConfirm;
        }
        Handheld::ManagementPresentation marketingPresentationFor(Page current, size_t selected) const
        {
            using Handheld::MarketingManagement::View;
            if (current == Page::marketing)
                return Handheld::ParkManagementTabs::Marketing(_marketingTabs.tab, selected, _status);
            const auto view = current == Page::marketing ? View::overview
                : current == Page::marketingActive       ? View::active
                : current == Page::marketingTypes        ? View::types
                : current == Page::marketingTargets      ? View::targets
                : current == Page::marketingEditor       ? View::editor
                                                         : View::confirmation;
            return Handheld::MarketingManagement::Presentation(
                view, selected, _marketingDraft, _marketingActiveType,
                view == View::overview ? _marketingActiveTypes : _marketingTypes, _marketingTargets, _status);
        }
        bool guestManagement() const
        {
            return _page == Page::guests || _page == Page::guestRoster || _page == Page::guestInspect
                || _page == Page::guestNeeds || _page == Page::guestThoughts || _page == Page::guestSpending
                || _page == Page::guestInventory || _page == Page::guestText || _page == Page::guestGroups
                || _page == Page::guestName || _page == Page::guestNameConfirm || _page == Page::guestTracking
                || _page == Page::guestPreferences || _page == Page::guestRideHistory || _page == Page::guestFilterName;
        }
        Handheld::ManagementPresentation guestPresentationFor(Page current, size_t selected) const
        {
            using Handheld::GuestManagement::View;
            if (current == Page::guestRideHistory && _guestTarget)
                return Handheld::GuestManagement::HistoryPresentation(*_guestTarget, _guestRides, selected);
            if (current == Page::guestFilterName)
            {
                auto model = Handheld::StaffManagement::NamePresentation(_nameEntry, selected, false, _status);
                model.breadcrumb = "Guests / Filter by name";
                model.title = "All guests";
                model.keyboardHeading = "Find guests by name";
                model.status = _status.empty() ? "Matches any part of a name, in any case" : _status;
                return model;
            }
            if (current == Page::guestName || current == Page::guestNameConfirm)
            {
                auto model = Handheld::StaffManagement::NamePresentation(
                    _nameEntry, selected, current == Page::guestNameConfirm, _status);
                model.breadcrumb = current == Page::guestName ? "Guests / Name keyboard" : "Guests / Review name";
                model.keyboardHeading = "Rename guest";
                return model;
            }
            if (current == Page::guestTracking && _guestTarget)
                return Handheld::GuestManagement::TrackingPresentation(*_guestTarget, _guestTrackingValue, selected);
            if (current == Page::guestGroups)
                return Handheld::GuestManagement::GroupPresentation(_guestGroupMode, _guestGroups, selected);
            const auto view = current == Page::guests ? View::overview
                : current == Page::guestRoster        ? View::roster
                : current == Page::guestPreferences   ? View::preferences
                : current == Page::guestInspect       ? View::inspection
                : current == Page::guestNeeds         ? View::needs
                : current == Page::guestThoughts      ? View::thoughts
                : current == Page::guestSpending      ? View::spending
                : current == Page::guestInventory     ? View::inventory
                                                      : View::text;
            auto model = Handheld::GuestManagement::Presentation(
                view, selected, _guestRoster, _guestTarget, _status, _guestText, _guestTextPage);
            if (current == Page::guestInspect && _guestTarget)
            {
                model = Handheld::PeopleTabs::Guest(*_guestTarget, _guestTabs, selected);
                model.status = _status;
                if (_guestTabs.tab == 0)
                    if (const auto* guest = Handheld::GuestManagement::Resolve(*_guestTarget))
                    {
                        model.hero = guestHero(*guest);
                        // The hero shows happiness and what they are doing; the cards keep the rest.
                        if (!model.bodyLines.empty())
                        {
                            model.hero->caption = model.bodyLines.front();
                            model.bodyLines.erase(model.bodyLines.begin());
                        }
                        std::erase_if(model.stats, [](const auto& stat) { return stat.first == "Happiness"; });
                    }
            }
            if (current == Page::guestRoster)
            {
                model.title = Handheld::GuestManagement::FilterName(_guestFilter);
                model.rows.back() = _guestRosterParent == Page::guestGroups ? "Back to groups"
                    : _guestRosterParent == Page::ridesCustomers            ? "Back to ride customers"
                                                                            : "Back to Guests";
                model.rows[2] = _guestFilter.tracked ? "All tracking" : "Tracked only";
                model.status = std::to_string(_guestRoster.size()) + " guests";
            }
            return model;
        }
        bool managementShell() const
        {
            return _page == Page::dashboard || financeManagement() || staffManagement() || researchManagement()
                || marketingManagement() || guestManagement() || ridesManagement() || overviewManagement();
        }
        std::optional<Handheld::ShopAppearance::Draft> _shopAppearanceDraft;
        size_t _shopAppearanceChannel{};
        bool _shopAppearanceBusy{};
        uint64_t _shopAppearanceRequest{};
        std::shared_ptr<int> _shopAppearanceLifetime = std::make_shared<int>(0);

    public:
        Handheld::ManagementPresentation shopAppearancePresentation() const
        {
            using namespace Handheld;
            ManagementPresentation model;
            model.breadcrumb = "Shops / Appearance";
            const auto* ride = _shop ? ShopManagement::Find(*_shop) : nullptr;
            model.title = ride ? ride->getName() : "Shop unavailable";
            model.selected = _focus.index();
            model.rows = rows();
            model.status = _status;
            if (_page == Page::shopAppearance)
            {
                if (ride)
                {
                    const auto* object = GetContext()->GetObjectManager().GetLoadedObject<RideObject>(ride->subtype);
                    model.bodyLines = { object ? object->GetName() : "Object unavailable" };
                }
                if (!_shop || ShopAppearance::Settings(*_shop).empty())
                    model.status = "This building has no colour options";
            }
            else if (_shopAppearanceDraft)
            {
                const auto& d = *_shopAppearanceDraft;
                model.stats = { { "Channel", d.setting.name },
                                { "Current", ShopAppearance::Label(d.setting, d.setting.current) } };
                if (_page == Page::shopAppearanceReview)
                {
                    model.confirmation = true;
                    model.title = "Apply shop appearance?";
                    model.stats.push_back({ "Proposed", ShopAppearance::Label(d.setting, d.value) });
                    const auto quote = ShopAppearance::Action(d, false);
                    model.stats.push_back(
                        { "Cost", quote.error == GameActions::Status::ok ? Handheld::FormatMoney(quote.cost) : "Unavailable" });
                    model.bodyLines = { "Changes only the selected building." };
                }
                else if (RideManagement::IsColourSetting(d.setting))
                {
                    model.colourValues = d.setting.values;
                    model.currentColour = d.setting.current;
                    model.title = d.setting.name;
                }
            }
            if (_shopAppearanceBusy)
                model.status = "Applying colour...";
            return model;
        }
        // A shop reads like a ride: its picture with Open/Closed as a switch, each product's price
        // changed in place with Left/Right (one native step, the same action the old editor sent),
        // then the less frequent actions as buttons. Rows: 0 status, 1..n prices, then the grid.
        size_t shopPriceCount() const
        {
            return _shop && Handheld::ShopManagement::Find(*_shop) ? Handheld::ShopManagement::Prices(*_shop).size() : 0;
        }
        Handheld::ManagementPresentation shopPresentation() const
        {
            using namespace Handheld;
            ManagementPresentation model;
            const auto* ride = _shop ? ShopManagement::Find(*_shop) : nullptr;
            model.title = ride ? ride->getName() : "Shop unavailable";
            model.rows = rows();
            model.selected = _focus.index();
            model.status = _status;
            if (!ride)
                return model;
            const bool noMoney = getGameState().park.flags.has(ParkFlag::noMoney);
            auto status = Formatter();
            ride->formatStatusTo(status);
            model.headerStatus = FormatStringIDLegacy(STR_STRINGID, status.Data());
            model.headerInk = ride->status == RideStatus::open ? "{POSITIVE}" : "{MUTED}";
            ManagementPresentation::Hero hero;
            const auto subtype = ride->subtype;
            hero.picture = [subtype](RenderTarget& rt) {
                if (const auto* object = GetContext()->GetObjectManager().GetLoadedObject(ObjectType::ride, subtype))
                {
                    rt.zoom_level = ZoomLevel{ 1 };
                    object->DrawPreview(rt, 128, 120);
                }
            };
            hero.states = { "Closed", "Open" };
            hero.state = ride->status == RideStatus::open ? 1 : 0;
            hero.caption = std::to_string(RideCustomersPerHour(*ride)) + " customers / hour";
            model.hero = hero;
            model.heroRow = 0;
            const auto prices = ShopManagement::Prices(*_shop);
            for (size_t i = 0; i < prices.size(); ++i)
            {
                model.settingRows.push_back(1 + i);
                // Some items (burgers, drinks) share one price across every stall that sells them.
                if (prices[i].shared)
                    model.rows[1 + i] = prices[i].name + "  (park-wide)";
                model.settingValues.push_back(noMoney ? "Free" : ShopManagement::Money(prices[i].value));
            }
            auto earnings = [noMoney](money64 value) {
                return noMoney                   ? std::string("-")
                    : value == kMoney64Undefined ? std::string("-")
                                                 : ShopManagement::Money(value);
            };
            model.stats = { { "Customers", std::to_string(ride->totalCustomers) },
                            { "Income / hr", earnings(ride->incomePerHour) },
                            { "Profit / hr", earnings(ride->profit) } };
            model.summaryMetrics = true;
            model.metricColumns = 3;
            model.metricHeight = 34;
            model.actionGrid = true;
            return model;
        }

    private:
        bool shopManagement() const
        {
            return _page == Page::shop || _page == Page::shopDemolish || _page == Page::shopAppearance
                || _page == Page::shopAppearanceEditor || _page == Page::shopAppearanceReview;
        }
        void pauseManagement()
        {
            if (!shopManagement())
                return;
            if (GameIsNotPaused())
            {
                PauseToggle();
                _pausedHere = true;
            }
            page(Page::park);
        }
        bool information() const
        {
            return _page == Page::overview || _page == Page::finances || _page == Page::guests || _page == Page::research
                || _page == Page::ride;
        }
        static std::string money(money64 value)
        {
            return Handheld::FormatMoney(value);
        }

    public:
        void parkMapSubjects()
        {
            if (_page != Page::parkMap)
                return;
            _mapSubjects = Handheld::MapSubjects::Roster(_parkMap.mode, _parkMap.filter, _parkMap.guestFilter);
            page(Page::mapSubjects);
            if (_mapTarget)
            {
                const auto found = std::find(_mapSubjects.begin(), _mapSubjects.end(), *_mapTarget);
                if (found != _mapSubjects.end())
                    _focus.move(static_cast<int>(found - _mapSubjects.begin()), rows().size());
            }
        }
        Handheld::ManagementPresentation mapSubjectsModel() const
        {
            return parkInformationPresentation();
        }
        CoordsXY mapFocus() const
        {
            return _parkMap.focus;
        }
        bool parkMapOpen() const
        {
            return _page == Page::parkMap;
        }
        void parkMapControl(int control)
        {
            if (!parkMapOpen())
                return;
            // 0 next view, 3 previous view (L1/R1, like tabs everywhere), 4 cycle zoom 1/2/4/8 (Y),
            // 2 details (R2); -1/+1 step zoom.
            if (control == 2)
                _parkMap.details = !_parkMap.details;
            else if (control == 0)
                _parkMap.Mode(1);
            else if (control == 3)
                _parkMap.Mode(-1);
            else if (control == 4)
                _parkMap.zoom = _parkMap.zoom >= 8 ? 1 : _parkMap.zoom * 2;
            else
                _parkMap.zoom = std::clamp(_parkMap.zoom + control, 1, 8);
            _parkMap.Update();
            invalidate();
        }
        Handheld::Accent accent() const
        {
            using Handheld::Accent;
            using Section = Handheld::Dashboard::Section;
            if (managementShell())
            {
                switch (_dashboard.section)
                {
                    case Section::rides:
                        return Accent::rides;
                    case Section::guests:
                        return Accent::guests;
                    case Section::staff:
                        return Accent::staff;
                    case Section::finances:
                        return Accent::finances;
                    case Section::research:
                        return Accent::research;
                    case Section::marketing:
                        return Accent::marketing;
                    default:
                        return Accent::standard;
                }
            }
            if (choiceCards() || picker())
                return Accent::build;
            return Accent::standard;
        }
        bool choiceCards() const
        {
            return _page == Page::build || _page == Page::paths || _page == Page::sceneryCategory
                || _page == Page::customChoice;
        }
        // Build menu art: a real loaded object of each kind, drawn with the same preview the object
        // catalog uses, so the big cards show this park's own rides, shops, paths and scenery.
        enum class BuildArt
        {
            ride,
            shop,
            path,
            scenery
        };
        static std::function<void(RenderTarget&)> BuildPicture(BuildArt kind)
        {
            auto& objects = GetContext()->GetObjectManager();
            const Object* chosen = nullptr;
            if (kind == BuildArt::ride || kind == BuildArt::shop)
            {
                for (ObjectEntryIndex i = 0; i < kMaxRideObjects && !chosen; ++i)
                    if (const auto* ride = objects.GetLoadedObject<RideObject>(i))
                        for (auto type : ride->GetEntry().ride_type)
                            if (type != kRideTypeNull)
                            {
                                const auto& rtd = GetRideTypeDescriptor(type);
                                const bool shop = rtd.flags.has(RtdFlag::isShopOrFacility);
                                if ((kind == BuildArt::shop) == shop
                                    && (shop || rtd.Category == RideCategory::rollerCoaster
                                        || rtd.Category == RideCategory::thrill))
                                    chosen = ride;
                                break;
                            }
            }
            else
            {
                const auto type = kind == BuildArt::path ? ObjectType::footpathSurface : ObjectType::smallScenery;
                for (ObjectEntryIndex i = 0; i < 64 && !chosen; ++i)
                    chosen = objects.GetLoadedObject(type, i);
            }
            if (!chosen)
                return {};
            const bool square = kind == BuildArt::ride || kind == BuildArt::shop;
            return [chosen, square](RenderTarget& rt) {
                // Ride previews are fixed 112 px squares drawn from their corner: centre a half-size
                // window for them. Other previews centre themselves.
                RenderTarget art = rt;
                if (square)
                {
                    const int side = std::min({ 56, rt.width, rt.height });
                    if (!ClipRenderTarget(art, rt, { (rt.width - side) / 2, (rt.height - side) / 2 }, side, side))
                        return;
                }
                art.zoom_level = ZoomLevel{ 1 };
                chosen->DrawPreview(art, art.zoom_level.ApplyTo(art.width), art.zoom_level.ApplyTo(art.height));
            };
        }
        std::vector<Handheld::ChoiceCard> choices() const
        {
            using I = Handheld::PanelIcon;
            if (_page == Page::build)
                // Grouped by what you are doing: building things first (big cards), then shaping
                // the land, then tools. Actions keep their native numbers; only the order changed.
                // "Park" is gone: B does that.
                return { { 8, "Rides", "Choose a ride, then place or construct it", I::play, SPR_TAB_RIDE_0, 0, 16,
                           BuildPicture(BuildArt::ride) },
                         { 7, "Shops", "Food, drinks, shops and guest facilities", I::folder, SPR_SHOP_ITEM_BURGER, 0, 1,
                           BuildPicture(BuildArt::shop) },
                         { 2, "Paths", "Footpaths and queues", I::path, 0, 0, 1, BuildPicture(BuildArt::path) },
                         { 10, "Scenery", "Trees, buildings, fences, signs and path furniture", I::largeScenery, 0, 0, 1,
                           BuildPicture(BuildArt::scenery) },
                         { 9, "Saved designs", "Rides you or others saved, ready to place", I::folder,
                           SPR_TOOLBAR_CONSTRUCT_RIDE, 0 },
                         { 0, "Raise hill", "Gentle hills; Y also digs, makes ponds or flattens", I::raise, 0, 1 },
                         { 1, "Dig", "Lower the ground with gentle slopes", I::lower, 0, 1 },
                         { 3, "Pond", "Dig a pond that fills with water", I::terrain, SPR_TAB_WATER, 1 },
                         { 4, "Surfaces", "Choose a ground surface", I::terrain, SPR_G2_TAB_LAND, 1 },
                         { 6, "Trees", "Plant from the tree catalog", I::tree, 0, 1 },
                         { 5, "Remove scenery", "Pick up placed scenery", I::remove, 0, 2 },
                         { 12, "Land & rights", "Buy land or construction rights", I::terrain, SPR_BUY_LAND_RIGHTS, 2 },
                         { 11, "Advanced", "Corners, edges, water levels and terrain painting", I::terrain, SPR_TOOLBAR_LAND,
                           2 } };
            if (_page == Page::paths)
                return { { 0, "Footpath", "Place individual tiles; Y Adjust also offers Line and Fill", I::path },
                         { 1, "Queue", "Build a queue for a ride entrance", I::path },
                         { 2, "Remove", "Remove a path tile under your cursor", I::remove },
                         { 4, "Extend path", "Build from an endpoint, including slopes and bridges", I::path },
                         { 5, "Extend queue", "Continue a queue from an endpoint", I::path },
                         { 3, "Build menu", "Return to the other building tools", I::back } };
            if (_page == Page::customChoice)
                return { { 1, "Ready-made design", "Pick a finished ride from the design catalog", I::folder },
                         { 0, "Build track", "Construct your own track, piece by piece", I::play, SPR_TAB_RIDE_0 },
                         { 2, "Back", "Return to the ride catalog", I::back } };
            // Each category shows a real object from it, drawn the way the catalog draws it, like
            // the Build menu's big cards. "Build menu" is gone: B does that.
            if (!_sceneryArtReady)
            {
                for (size_t c = 0; c < _sceneryArt.size(); ++c)
                {
                    using SC = Handheld::SceneryTools::Category;
                    const auto category = static_cast<SC>(c);
                    const auto entries = Handheld::SceneryTools::Entries(category);
                    if (entries.empty())
                        continue;
                    // A representative object rather than whatever loaded first: a tree, the
                    // biggest building, a bench or lamp.
                    auto entry = entries.front();
                    auto named = [&](std::initializer_list<const char*> words) {
                        for (const auto& candidate : entries)
                            for (const auto* word : words)
                                if (candidate.name.find(word) != std::string::npos)
                                {
                                    entry = candidate;
                                    return;
                                }
                    };
                    if (category == SC::small)
                        named({ "Tree", "tree" });
                    else if (category == SC::additions)
                        named({ "Bench", "Lamp", "bench", "lamp" });
                    else if (category == SC::large)
                    {
                        size_t most = 0;
                        for (const auto& candidate : entries)
                            if (const auto* large = ObjectEntryManager::GetObjectEntry<LargeSceneryEntry>(
                                    candidate.identity.index))
                                if (large->tiles.size() > most)
                                {
                                    most = large->tiles.size();
                                    entry = candidate;
                                }
                    }
                    _sceneryArt[c] = [entry](RenderTarget& rt) {
                        if (entry.drawPreview)
                        {
                            entry.drawPreview(rt);
                            return;
                        }
                        if (const auto* object = GetContext()->GetObjectManager().GetLoadedObject(
                                entry.identity.type, entry.identity.index))
                        {
                            RenderTarget art = rt;
                            art.zoom_level = ZoomLevel{ 1 };
                            object->DrawPreview(art, art.zoom_level.ApplyTo(art.width), art.zoom_level.ApplyTo(art.height));
                        }
                    };
                }
                _sceneryArtReady = true;
            }
            return { { 0, "Trees & small", "Plants, trees and small scenery objects", I::tree, 0, 0, 1, _sceneryArt[0] },
                     { 1, "Buildings", "Large scenery and building objects", I::largeScenery, 0, 0, 1, _sceneryArt[1] },
                     { 2, "Walls & fences", "Choose a wall or fence", I::walls, 0, 0, 1, _sceneryArt[2] },
                     { 3, "Signs", "Banners and signs", I::furniture, 0, 0, 1, _sceneryArt[3] },
                     { 4, "Path furniture", "Benches, bins, lamps and other path additions", I::furniture, 0, 0, 1,
                       _sceneryArt[4] },
                     { 5, "Clear area", "Select which scenery types to clear with a brush", I::remove, SPR_DEMOLISH } };
        }
        mutable std::array<std::function<void(RenderTarget&)>, 5> _sceneryArt;
        mutable bool _sceneryArtReady{};
        bool constructionMenu() const
        {
            return choiceCards() || _page == Page::controls || _page == Page::primer || _page == Page::park
                || _page == Page::settings || _page == Page::designOptions || _page == Page::researchCatalog
                || _page == Page::financeHistory || _page == Page::financeLedger || _page == Page::parkMap
                || _page == Page::peepOptions || _page == Page::peepCancel || _page == Page::staffPatrolOptions
                || _page == Page::staffPatrolClear || _page == Page::rightsOptions || _page == Page::rightsConfirm
                || (_page == Page::parkFiles || _page == Page::saveFiles || _page == Page::designFiles)
                || _page == Page::areaOptions || _page == Page::terrainPaintConfirm || _page == Page::sceneryOptions
                || _page == Page::parkInspect
                || (_page == Page::designInspect || _page == Page::designVehicles || _page == Page::designWarnings
                    || _page == Page::designConfirm)
                || _page == Page::saveName || _page == Page::saveReview || _page == Page::sceneryName
                || _page == Page::scenerySearch || _page == Page::sceneryConfirm || _page == Page::sceneryGroups
                || _page == Page::sceneryStyle || _page == Page::sceneryStyleEditor || _page == Page::sceneryStyleConfirm
                || _page == Page::sceneryClearOptions || _page == Page::sceneryClearConfirm;
        }
        bool toolSettingsPage(int direction)
        {
            if (_page != Page::areaOptions && _page != Page::sceneryOptions)
                return false;
            const bool scenery = _page == Page::sceneryOptions;
            const auto groups = Handheld::ToolSettingGroups(scenery);
            const auto group = Handheld::ToolSettingGroup(scenery, _focus.index());
            _toolGroupRows[scenery][group] = _focus.index();
            const auto next = (static_cast<int>(group) + direction + groups.size()) % groups.size();
            const auto saved = _toolGroupRows[scenery][next];
            const auto row = std::find(groups[next].begin(), groups[next].end(), saved) != groups[next].end()
                ? saved
                : groups[next].front();
            _focus.move(static_cast<int>(row) - static_cast<int>(_focus.index()), rows().size());
            _status.clear();
            invalidate();
            return true;
        }
        void constructionMove(int x, int y)
        {
            if (_page == Page::designWarnings && !_designTechnical)
            {
                _focus.move(x ? x : y, rows().size());
                invalidate();
                return;
            }
            if (y && (_page == Page::areaOptions || _page == Page::sceneryOptions))
            {
                const bool scenery = _page == Page::sceneryOptions;
                const auto indices = Handheld::ToolSettingGroups(scenery)[Handheld::ToolSettingGroup(scenery, _focus.index())];
                const auto slot = std::find(indices.begin(), indices.end(), _focus.index()) - indices.begin();
                const auto next = std::clamp<int>(static_cast<int>(slot) + y, 0, static_cast<int>(indices.size()) - 1);
                _focus.move(static_cast<int>(indices[next]) - static_cast<int>(_focus.index()), rows().size());
                _status.clear();
                invalidate();
                return;
            }
            if (_page == Page::controls)
            {
                _focus.move(x ? x : y, Handheld::Guide::kTabs.size());
                invalidate();
                return;
            }
            if (_page == Page::primer)
            {
                // Cards step without wrapping: Right past the last card does nothing.
                const int step = x ? x : y;
                const int next = std::clamp<int>(
                    static_cast<int>(_focus.index()) + step, 0, static_cast<int>(rows().size()) - 1);
                _focus.move(next - static_cast<int>(_focus.index()), rows().size());
                invalidate();
                return;
            }
            if (_page == Page::park && x)
            {
                // Two columns: Right/Left cross between Game and the speed / display column at the
                // same height (rows 0-7 and 8-14).
                const size_t i = _focus.index();
                const size_t target = x > 0 ? (i < 8 ? std::min<size_t>(8 + i, 14) : i)
                                            : (i >= 8 ? std::min<size_t>(i - 8, 7) : i);
                _focus.move(static_cast<int>(target) - static_cast<int>(i), rows().size());
                invalidate();
                return;
            }
            if (_page == Page::settings)
            {
                if (y)
                    move(y);
                else if (x)
                    toggleSetting(_focus.index());
                invalidate();
                return;
            }
            if (choiceCards())
            {
                const auto next = _page == Page::build ? Handheld::MoveChoiceGroups(choices(), 3, _focus.index(), x, y)
                                                       : Handheld::MoveChoiceCards(choices(), _focus.index(), 3, x, y);
                _focus.move(static_cast<int>(next) - static_cast<int>(_focus.index()), rows().size());
                invalidate();
                return;
            }
            if (_page == Page::designOptions && x)
            {
                const auto row = _focus.index();
                if (row == 1)
                    Handheld::Designs::Height(x);
                else if (row == 2)
                {
                    if (Handheld::Designs::HeightOffset())
                        Handheld::Designs::AutomaticHeight();
                    else
                        Handheld::Designs::Height(1);
                }
                else if (row == 3)
                    for (int i = 0; i < (x > 0 ? 1 : 3); ++i)
                        Handheld::Designs::Rotate();
                else if (row == 4)
                    Handheld::Designs::SetScenery(!Handheld::Designs::IncludesScenery());
                refreshDesignWorkspace();
                invalidate();
                return;
            }
            if (_page == Page::designVehicles)
            {
                auto picker = designVehiclePicker();
                picker.move(x, y);
                _focus.move(static_cast<int>(picker.focus()) - static_cast<int>(_focus.index()), rows().size());
                invalidate();
                return;
            }
            if ((_page == Page::designInspect || _page == Page::designVehicles || _page == Page::designWarnings) && x)
            {
                const auto labels = rows();
                const auto text = _focus.index() < labels.size() ? labels[_focus.index()] : std::string{};
                _designDetail = static_cast<size_t>(std::clamp<int64_t>(
                    static_cast<int64_t>(_designDetail) + x, 0, Handheld::FileBrowsing::DetailPages(text) - 1));
                invalidate();
                return;
            }
            if ((_page == Page::designInspect || _page == Page::designVehicles || _page == Page::designWarnings) && y)
                _designDetail = 0;
            if ((_page == Page::parkFiles || _page == Page::saveFiles || _page == Page::designFiles) && x)
            {
                _fileDetail = static_cast<size_t>(std::clamp<int64_t>(
                    static_cast<int64_t>(_fileDetail) + x, 0, Handheld::FileBrowsing::DetailPages(fileDetailPath()) - 1));
                invalidate();
                return;
            }
            if (_page == Page::researchCatalog)
            {
                auto picker = Handheld::ResearchCatalog::Picker(_researchCatalog, _focus.index());
                picker.move(x, y);
                _focus.move(static_cast<int>(picker.focus()) - static_cast<int>(_focus.index()), rows().size());
                invalidate();
                return;
            }
            if (_page == Page::financeHistory)
            {
                if (x)
                    _financeSample = std::clamp<int>(static_cast<int>(_financeSample) - x, 0, 63);
                if (y)
                    move(y);
                invalidate();
                return;
            }
            if (_page == Page::financeLedger)
            {
                if (x)
                    _ledger.Move(x);
                if (y)
                    move(y);
                invalidate();
                return;
            }
            if (_page == Page::parkMap)
            {
                _parkMap.Move(x, y);
                invalidate();
                return;
            }
            if (_page == Page::saveName || _page == Page::sceneryName || _page == Page::scenerySearch)
            {
                const auto next = _nameEntry.Move(_focus.index(), x, y);
                _focus.move(static_cast<int>(next) - static_cast<int>(_focus.index()), rows().size());
            }
            else if (y)
                move(y);
            else if (x && _page == Page::staffPatrolOptions)
            {
                Handheld::StaffPatrol::Configure(_focus.index(), x);
                invalidate();
            }
            else if (x && _page == Page::rightsOptions)
                Handheld::RightsTools::Configure(_focus.index(), x);
            else if (x && _page == Page::areaOptions)
            {
                if (Handheld::ConstructionAdjust::TerrainEnabled(_focus.index()))
                    Handheld::AreaTools::Configure(_focus.index(), x);
                else
                    _status = "Fixed or unavailable for this tool";
            }
            else if (x && _page == Page::sceneryClearOptions)
                Handheld::SceneryClear::Configure(_focus.index(), x);
            else if (x && _page == Page::sceneryStyleEditor && _focus.index() == 0 && _signStyle)
                Handheld::SceneryTools::AdjustStyle(*_signStyle, x);
            else if (x && _page == Page::sceneryOptions)
            {
                if (Handheld::ConstructionAdjust::SceneryEnabled(_focus.index()))
                    Handheld::SceneryTools::Configure(_focus.index(), x);
                else
                    _status = "Fixed or unavailable for this object";
            }
            invalidate();
        }

    private:
        // Pages that draw across the whole canvas. The window must cover what they draw, or the
        // parts outside it are never repainted (stale highlights and toggles).
        bool fullScreen() const
        {
            return choiceCards() || _page == Page::staffPatrolClear || _page == Page::rightsConfirm
                || _page == Page::parkInspect || _page == Page::designInspect || _page == Page::designVehicles
                || _page == Page::designWarnings || _page == Page::designConfirm || _page == Page::saveName
                || _page == Page::saveReview || _page == Page::sceneryName || _page == Page::scenerySearch
                || _page == Page::sceneryConfirm || _page == Page::parkFiles || _page == Page::saveFiles
                || _page == Page::designFiles || _page == Page::parkMap || parkInformation() || _page == Page::pathOptions
                || picker() || shopManagement() || managementShell() || _page == Page::park || _page == Page::settings
                || _page == Page::controls || _page == Page::primer || _page == Page::save || _page == Page::load;
        }
        void layout()
        {
            auto size = Handheld::OverlaySize(std::min<size_t>(8, rows().size()));
            if (_page == Page::park || _page == Page::quitChoice || _page == Page::exit)
                size.height += 16;
            // Confirmation text needs room; normal tools leave the centre cursor exposed.
            const ScreenSize target{ _page == Page::overwrite || _page == Page::discard || _page == Page::exit
                                             || _page == Page::quitChoice
                                         ? 330
                                         : size.width,
                                     size.height };
            // Information sheets are sized for a 4:3 screen; on a shorter one (16:9 handhelds) they
            // stop above the footer instead of running underneath it.
            const int footerTop = ContextGetHeight() - Handheld::Layout::actionHeight - Handheld::Layout::margin;
            const ScreenSize bounds = fullScreen() ? ScreenSize{ ContextGetWidth(), ContextGetHeight() }
                : information()                    ? ScreenSize{ 330, std::min(_page == Page::ride ? 340 : 310, footerTop - 8) }
                                                   : target;
            WindowSetResize(*this, bounds, bounds);
            WindowSetPosition(
                *this,
                fullScreen()
                    ? ScreenCoordsXY{ 0, 0 }
                    : ScreenCoordsXY{
                          8, std::clamp((ContextGetHeight() - bounds.height) / 2, 8, std::max(8, footerTop - bounds.height)) });
        }

        std::vector<Handheld::ControlHint> _guideHints = Handheld::LastControlHints;
        std::function<void()> _leaveAction;
        size_t _leaveFocus{};
        bool _leaveStaff{};
        bool researchDirty() const
        {
            return _page == Page::research && _researchTabs.tab == 1 && _researchDraft
                && (_researchDraft->funding != _researchDraft->baselineFunding
                    || _researchDraft->priorities != _researchDraft->baselinePriorities);
        }
        bool dutiesDirty() const
        {
            return _page == Page::staffInspect && _staffTabs.tab == 1 && _staffTarget
                && Handheld::StaffManagement::Resolve(*_staffTarget) && _ordersDraft != _ordersBaseline;
        }
        bool guardLeave(std::function<void()> action)
        {
            if (_leaveAction)
                return true;
            if (!researchDirty() && !dutiesDirty())
                return false;
            _leaveFocus = _focus.index();
            _leaveStaff = dutiesDirty();
            _leaveAction = std::move(action);
            _focus = {};
            _status.clear();
            invalidate();
            return true;
        }
        Handheld::ManagementPresentation leavePresentation() const
        {
            Handheld::ManagementPresentation m;
            m.breadcrumb = "Unsaved changes";
            m.title = _leaveStaff ? "Keep duty changes?" : "Keep research changes?";
            m.confirmation = true;
            m.headerInk = "{CAUTION}";
            m.headerStatus = "Unsaved";
            m.bodyLines = { "Your park has not changed.", "Apply to save, or discard only these edits." };
            m.rows = { "Continue editing", "Apply changes", "Discard changes" };
            m.rowEnabled = std::vector<bool>(3, !_civicBusy);
            m.selected = _focus.index();
            m.status = _civicBusy ? "Applying changes..." : _status;
            return m;
        }
        void finishLeave(bool discard)
        {
            if (discard)
            {
                if (_leaveStaff && _staffTarget)
                {
                    if (const auto* staff = Handheld::StaffManagement::Resolve(*_staffTarget))
                        _ordersBaseline = _ordersDraft = staff->staffOrders;
                }
                else
                    _researchDraft = Handheld::ResearchManagement::Begin();
            }
            auto action = std::move(_leaveAction);
            _leaveAction = {};
            _focus = {};
            _focus.move(static_cast<int>(_leaveFocus), rows().size());
            _status.clear();
            if (action)
                action();
        }
        void confirmLeave()
        {
            if (_civicBusy)
                return;
            if (_focus.index() == 0)
            {
                cancelLeave();
                return;
            }
            if (_focus.index() == 2)
            {
                finishLeave(true);
                return;
            }
            _civicBusy = true;
            auto callback = [this,
                             lifetime = std::weak_ptr<int>(_civicLifetime)](const auto*, const GameActions::Result* outcome) {
                if (lifetime.expired())
                    return;
                _civicBusy = false;
                if (!_leaveAction)
                    return;
                if (outcome->error == GameActions::Status::ok)
                    finishLeave(true);
                else
                {
                    _status = Handheld::Finances::Feedback(*outcome);
                    invalidate();
                }
            };
            const auto result = _leaveStaff
                ? Handheld::StaffManagement::SetOrders(*_staffTarget, _ordersBaseline, _ordersDraft, true, callback)
                : Handheld::ResearchManagement::Action(*_researchDraft, true, callback);
            if (result.error != GameActions::Status::ok)
            {
                _civicBusy = false;
                _status = Handheld::Finances::Feedback(result);
            }
            invalidate();
        }
        void cancelLeave()
        {
            if (_civicBusy)
                return;
            _leaveAction = {};
            _focus = {};
            _focus.move(static_cast<int>(_leaveFocus), rows().size());
            _status.clear();
            invalidate();
        }

        void page(Page next)
        {
            if (next != _page && next != Page::researchConfirm && guardLeave([this, next] { page(next); }))
                return;
            _pageRows[_page] = _focus.index();
            if (next == Page::sceneryPicker && _page != Page::scenerySearch)
                _sceneryPickerDirect = false;
            if (_page == Page::build)
                _buildFocus = _focus.index();
            if (_page == Page::paths)
                _pathModeFocus = _focus.index();
            if (_page == Page::pathOptions)
                _pathOptionsFocus[Handheld::Paths::IsConnected()] = _focus.index();
            if (next == Page::park || next == Page::dashboard)
                _managementNewsReturn = false;
            if (_paintReviewResumed && next != Page::terrainPaintConfirm)
            {
                if (_pausedHere && !(gGamePaused & GAME_PAUSED_NORMAL))
                    PauseToggle();
                _paintReviewResumed = false;
            }
            if (_clearReviewResumed && next != Page::sceneryClearConfirm)
            {
                if (_pausedHere && !(gGamePaused & GAME_PAUSED_NORMAL))
                    PauseToggle();
                _clearReviewResumed = false;
            }
            if (_pathUndoResumed && next != Page::pathUndoConfirm)
            {
                if (_pausedHere && !(gGamePaused & GAME_PAUSED_NORMAL))
                    PauseToggle();
                _pathUndoResumed = false;
            }
            if (_rightsReviewResumed && next != Page::rightsConfirm)
            {
                if (_pausedHere && !(gGamePaused & GAME_PAUSED_NORMAL))
                    PauseToggle();
                _rightsReviewResumed = false;
            }

            if (_ridesReviewResumed && next != Page::ridesConfirm)
            {
                // Return only our Dashboard pause; never clear native/external pause reasons.
                if (_pausedHere && !(gGamePaused & GAME_PAUSED_NORMAL))
                    PauseToggle();
                _ridesReviewResumed = false;
            }
            if (next != Page::designConfirm)
                Handheld::Designs::CancelReview();
            if (next != Page::overviewConfirm)
                _overviewDraft.reset();
            _overviewBusy = false;
            ++_overviewRequest;
            if (_page == Page::finances)
                _financeTabs.save(_focus.index());
            if (_page == Page::research)
                _researchTabs.save(_focus.index());
            if (_page == Page::marketing)
                _marketingTabs.save(_focus.index());
            if (_page == Page::overview)
                _parkTabs.save(_focus.index());
            if (_page == Page::staffInspect)
                _staffTabs.save(_focus.index());
            if (_page == Page::guestInspect)
                _guestTabs.save(_focus.index());
            if (_page == Page::ridesInspect)
            {
                _ridesInspectFocus = _focus.index();
                _rideTabs.save(_focus.index());
            }
            if (_page == Page::ridesRoster)
                _ridesRosterFocus = _focus.index();
            if (next == Page::ridesRoster)
                _ridesRoster = Handheld::RideManagement::Roster();
            if (next != Page::ridesPrice && next != Page::ridesConfirm && next != Page::ridesName)
                _ridesDraft.reset();
            _ridesBusy = false;
            ++_ridesRequest;
            if (_page == Page::guestRoster)
                _guestRosterFocus = _focus.index();
            if (next == Page::dashboard && _page != Page::dashboard && managementShell() && dashboardCanSwitch())
                _sectionResume[_dashboard.index()] = { _page, _focus.index() };
            _staffBusy = false;
            ++_staffRequest;
            if (next == Page::guestRideHistory)
                _guestRides = _guestTarget ? Handheld::GuestManagement::RideHistory(*_guestTarget)
                                           : std::vector<Handheld::RideManagement::Target>{};
            if (next == Page::guestGroups)
                _guestGroups = Handheld::GuestManagement::Groups(_guestGroupMode);
            if (next == Page::guestRoster)
                _guestRoster = Handheld::GuestManagement::Roster(_guestFilter);
            if (next == Page::staff)
                _staffRoster = Handheld::StaffManagement::Roster(_staffFilter);
            if (next == Page::marketing)
                _marketingActiveTypes = Handheld::MarketingManagement::ActiveTypes();
            if (next == Page::marketingTypes)
                _marketingTypes = Handheld::MarketingManagement::Types();
            if (next == Page::marketingTargets && _marketingDraft)
                _marketingTargets = Handheld::MarketingManagement::Targets(_marketingDraft->type);
            if (next != Page::marketingEditor && next != Page::marketingTargets && next != Page::marketingConfirm)
                _marketingDraft.reset();
            if ((_page == Page::designName || _page == Page::designScenery || _page == Page::designExport)
                && next != Page::designName && next != Page::designScenery && next != Page::designExport)
                Handheld::DesignAuthoring::End();
            if (next != Page::shopAppearanceEditor && next != Page::shopAppearanceReview)
                _shopAppearanceDraft.reset();
            _shopAppearanceBusy = false;
            ++_shopAppearanceRequest;
            _page = next;
            if (next == Page::save || next == Page::load)
                AdoptSharedSlots();
            if (next == Page::save || next == Page::load)
                for (size_t slot = 0; slot < 3; ++slot)
                    _slotPreviews[slot] = Handheld::SavedParkPresentation::Read(SlotPath(slot));
            if (next == Page::designOptions)
                refreshDesignWorkspace();
            if (next == Page::ridesName || next == Page::designName || next == Page::designScenery
                || next == Page::designExport)
            {
                _dashboard.focus = Handheld::Dashboard::Focus::content;
                _dashboard.section = Handheld::Dashboard::Section::rides;
            }
            if (next == Page::dashboard)
            {
                _dashboard.focus = Handheld::Dashboard::Focus::rail;
                _objectivesFromPark = false;
            }
            else if (next == Page::staff)
                _dashboard.section = Handheld::Dashboard::Section::staff;
            else if (next == Page::finances)
                _dashboard.section = Handheld::Dashboard::Section::finances;
            if (next == Page::overview)
                _dashboard.section = Handheld::Dashboard::Section::overview;
            if (next == Page::ridesRoster)
                _dashboard.section = Handheld::Dashboard::Section::rides;
            if (next == Page::guests)
                _dashboard.section = Handheld::Dashboard::Section::guests;
            if (next == Page::marketing)
                _dashboard.section = Handheld::Dashboard::Section::marketing;
            if (next == Page::research)
                _dashboard.section = Handheld::Dashboard::Section::research;
            if (next != Page::researchEditor && next != Page::researchConfirm
                && !(next == Page::research && _researchTabs.tab == 1))
                _researchDraft.reset();
            if (next == Page::research && _researchTabs.tab == 1 && !_researchDraft)
                _researchDraft = Handheld::ResearchManagement::Begin();
            if (next != Page::financeEditor && next != Page::financeConfirm)
                _financeDraft.reset();
            _focus = {};
            const bool restorePage = next == Page::ridesOperations || next == Page::ridesVehicles || next == Page::ridesDispatch
                || next == Page::ridesMaintenance || next == Page::ridesAppearance || next == Page::ridesMusic
                || next == Page::shopAppearance || next == Page::designOptions || next == Page::designInspect
                || next == Page::designWarnings || next == Page::designVehicles || next == Page::trackBlock
                || next == Page::areaOptions || next == Page::sceneryOptions;
            if (const auto count = rows().size(); restorePage && count && _pageRows.count(next))
                _focus.move(static_cast<int>(std::min(_pageRows.at(next), count - 1)), count);
            if (next == Page::ridesPrice && _ridesDraft && _ridesDraft->setting
                && (_ridesDraft->setting->appearance || rideVehicleChoice()))
            {
                const auto& values = _ridesDraft->setting->values;
                const auto selected = std::find(values.begin(), values.end(), _ridesDraft->value);
                if (selected != values.end())
                    _focus.move(static_cast<int>(selected - values.begin()), values.size());
            }
            if (next == Page::finances)
                _focus.move(static_cast<int>(std::min(_financeTabs.rows[_financeTabs.tab], rows().size() - 1)), rows().size());
            if (next == Page::research)
                _focus.move(
                    static_cast<int>(std::min(_researchTabs.rows[_researchTabs.tab], rows().size() - 1)), rows().size());
            if (next == Page::marketing)
                _focus.move(
                    static_cast<int>(std::min(_marketingTabs.rows[_marketingTabs.tab], rows().size() - 1)), rows().size());
            if (next == Page::overview)
                _focus.move(static_cast<int>(std::min(_parkTabs.rows[_parkTabs.tab], rows().size() - 1)), rows().size());
            if (next == Page::staffInspect)
                _focus.move(static_cast<int>(std::min(_staffTabs.rows[_staffTabs.tab], rows().size() - 1)), rows().size());
            if (next == Page::guestInspect)
                _focus.move(static_cast<int>(std::min(_guestTabs.rows[_guestTabs.tab], rows().size() - 1)), rows().size());
            if (next == Page::ridesInspect)
                _focus.move(static_cast<int>(std::min(_rideTabs.rows[_rideTabs.tab], rows().size() - 1)), rows().size());
            if (next == Page::build)
                _focus.move(static_cast<int>(_buildFocus), rows().size());
            if (next == Page::paths)
                _focus.move(static_cast<int>(_pathModeFocus), rows().size());
            if (next == Page::pathOptions)
                _focus.move(static_cast<int>(_pathOptionsFocus[Handheld::Paths::IsConnected()]), rows().size());
            _status.clear();
            layout();
            if (next == Page::parkFiles || next == Page::saveFiles || next == Page::designFiles)
                _focus.move(static_cast<int>(_fileFocus), rows().size());
            if (next == Page::staff)
                _focus.move(static_cast<int32_t>(std::min(_staffRosterFocus, rows().size() - 1)), rows().size());

            if (next == Page::trackBlocks && _trackBlock)
            {
                const auto found = std::find(_trackBlocks.begin(), _trackBlocks.end(), *_trackBlock);
                if (found != _trackBlocks.end())
                    _focus.move(static_cast<int>(found - _trackBlocks.begin()), rows().size());
            }
            if (next == Page::ridesRoster)
                _focus.move(static_cast<int32_t>(std::min(_ridesRosterFocus, rows().size() - 1)), rows().size());
            if (next == Page::guestGroups)
                _focus.move(static_cast<int32_t>(std::min(_guestGroupFocus, rows().size() - 1)), rows().size());
            if (next == Page::guestRideHistory)
                _focus.move(static_cast<int32_t>(std::min(_guestRideFocus, rows().size() - 1)), rows().size());
            if (next == Page::guestRoster)
                _focus.move(static_cast<int32_t>(std::min(_guestRosterFocus, rows().size() - 1)), rows().size());
            if (next == Page::customChoice)
                _focus.move(1, rows().size()); // Ready-made designs are the quickest way to a coaster.
            // Only confirmations that lose something (a save, the session, a ride, an employee, a
            // placement) start on Cancel; ordinary decisions start on the action so A-A completes them.
            const bool destructive = next == Page::overwrite || next == Page::discard || next == Page::exit
                || next == Page::shopDemolish || next == Page::staffDismiss || next == Page::peepCancel
                || (next == Page::ridesConfirm && _ridesDraft && _ridesDraft->demolition)
                || (next == Page::saveReview && _saveDraft && _saveDraft->baseline);
            if (destructive)
                _focus.move(1, 2);
            invalidate();
        }
        std::vector<Handheld::SceneryTools::Style> signStyles() const
        {
            std::vector<Handheld::SceneryTools::Style> result;
            if (_signTarget)
                for (auto style : { Handheld::SceneryTools::Style::noEntry, Handheld::SceneryTools::Style::textColour,
                                    Handheld::SceneryTools::Style::mainColour })
                    if (Handheld::SceneryTools::BeginStyle(*_signTarget, style))
                        result.push_back(style);
            return result;
        }
        std::string simulationChoice(uint8_t speed) const
        {
            const bool paused = (gGamePaused & ~(_pausedHere ? GAME_PAUSED_NORMAL : 0)) != 0;
            const bool current = speed == 0 ? paused : !paused && gGameSpeed == speed;
            return std::string(current ? "Current: " : "") + (speed == 0 ? "Paused" : std::to_string(1u << (speed - 1)) + "x");
        }
        void chooseSimulation(uint8_t speed)
        {
            if (speed != 0)
            {
                GameActions::GameSetSpeedAction action(speed);
                const auto result = GameActions::Execute(&action, getGameState());
                if (result.error != GameActions::Status::ok)
                {
                    _status = result.getErrorMessage();
                    invalidate();
                    return;
                }
            }
            const bool normalPause = gGamePaused & GAME_PAUSED_NORMAL;
            if (normalPause != (speed == 0))
                PauseToggle();
            _pausedHere = false;
            close();
        }
        void cancelQuit()
        {
            _quitSaving = false;
            _exitAfterSave = false;
            if (_quitStandalone)
            {
                close();
                return;
            }
            page(_quitCaller);
            _focus = {};
            _focus.move(static_cast<int>(_quitFocus), rows().size());
        }
        std::vector<std::string> rows() const
        {
            if (_leaveAction)
                return leavePresentation().rows;
            switch (_page)
            {
                case Page::park:
                    return { "Resume park",
                             "Build",
                             "Park dashboard",
                             "Save park",
                             "Save and quit",
                             "Load park",
                             "Quit...",
                             "Controls...",
                             simulationChoice(0),
                             simulationChoice(1),
                             simulationChoice(2),
                             simulationChoice(3),
                             Handheld::MenuThemes::Choice(),
                             Handheld::MenuThemes::MotionChoice(),
                             "More settings..." };
                case Page::quitChoice:
                    return { "Save and quit", "Quit without saving...", "Cancel" };
                case Page::controls:
                    return { Handheld::Guide::kTabs.begin(), Handheld::Guide::kTabs.end() };
                case Page::primer:
                {
                    std::vector<std::string> cards;
                    for (const auto& card : Handheld::Guide::Cards())
                        cards.push_back(card.title);
                    return cards;
                }
                case Page::settings:
                    return { "Theme",
                             "Motion",
                             "Smart cursor",
                             "Goal in the HUD",
                             "Clock and battery in the HUD",
                             "Continue last park on launch",
                             "Save when the device sleeps" };
                case Page::dashboard:
                    return HandheldDashboardPresentation(0).rows;
                case Page::staff:
                case Page::staffInspect:
                case Page::staffStats:
                case Page::staffOrders:
                case Page::staffHire:
                case Page::staffDismiss:
                case Page::staffRoles:
                case Page::staffCostumes:
                case Page::staffName:
                case Page::staffNameConfirm:
                case Page::staffSummary:
                case Page::staffUniform:
                    return staffPresentation().rows;
                case Page::finances:
                    return financeWorkspace().rows;
                case Page::financeEditor:
                    return { "Review change...", "Cancel" };
                case Page::financeConfirm:
                    return { "Apply", "Cancel" };
                case Page::researchCatalog:
                case Page::researchItem:
                case Page::financeHistory:
                case Page::financeLedger:
                case Page::mapSubjects:
                case Page::mapSubject:
                case Page::parkNews:
                case Page::parkNewsDetail:
                case Page::parkAwards:
                case Page::parkHistory:
                    return parkInformationPresentation().rows;
                case Page::parkMap:
                    return { "Locate tile" };
                case Page::overview:
                case Page::overviewConfirm:
                    return overviewPresentation().rows;
                case Page::trackBlocks:
                case Page::trackContext:
                case Page::trackBlock:
                case Page::trackRemoveConfirm:
                case Page::trackReplaceConfirm:
                case Page::trackSpeedConfirm:
                case Page::ridesRoster:
                case Page::ridesInspect:
                case Page::accessManage:
                case Page::accessPoint:
                case Page::accessReview:
                case Page::ridesAccess:
                case Page::ridesVehicles:
                case Page::ridesName:
                case Page::designName:
                case Page::designScenery:
                case Page::designExport:
                case Page::ridesAppearance:
                case Page::ridesMusic:
                case Page::ridesDispatch:
                case Page::ridesOperations:
                case Page::ridesMaintenance:
                case Page::ridesGraphs:
                case Page::ridesStations:
                case Page::ridesCustomers:
                case Page::ridesEarnings:
                case Page::ridesStats:
                case Page::ridesPrice:
                case Page::ridesConfirm:
                    return ridesPresentationFor(_page, _focus.index()).rows;
                case Page::guests:
                case Page::guestRoster:
                case Page::guestInspect:
                case Page::guestNeeds:
                case Page::guestThoughts:
                case Page::guestSpending:
                case Page::guestInventory:
                case Page::guestText:
                case Page::guestGroups:
                case Page::guestName:
                case Page::guestFilterName:
                case Page::guestNameConfirm:
                case Page::guestTracking:
                case Page::guestPreferences:
                case Page::guestRideHistory:
                    return guestsPresentation().rows;
                case Page::marketing:
                case Page::marketingActive:
                case Page::marketingTypes:
                case Page::marketingEditor:
                case Page::marketingTargets:
                case Page::marketingConfirm:
                    return marketingPresentationFor(_page, _focus.index()).rows;
                case Page::research:
                case Page::researchEditor:
                case Page::researchConfirm:
                    return researchPresentation().rows;
                case Page::shop:
                {
                    std::vector<std::string> result{ "Status" };
                    if (_shop && Handheld::ShopManagement::Find(*_shop))
                        for (const auto& price : Handheld::ShopManagement::Prices(*_shop))
                            result.push_back(price.name);
                    result.insert(result.end(), { "Demolish...", "Rename...", "Appearance..." });
                    return result;
                }
                case Page::shopAppearance:
                {
                    std::vector<std::string> result;
                    if (_shop)
                        for (const auto& setting : Handheld::ShopAppearance::Settings(*_shop))
                            result.push_back(setting.name + " / " + Handheld::ShopAppearance::Label(setting, setting.current));
                    result.push_back("Back to shop");
                    return result;
                }
                case Page::shopAppearanceEditor:
                    return _shopAppearanceDraft ? _shopAppearanceDraft->setting.labels
                                                : std::vector<std::string>{ "Shop unavailable" };
                case Page::shopAppearanceReview:
                    return { "Cancel", "Apply change" };
                case Page::shopDemolish:
                    return { "Demolish", "Cancel" };
                case Page::ride:
                {
                    std::vector<std::string> result{ "Open ride", "Close ride", "Back to park" };
                    if (_ridesTarget && Handheld::FlatRides::CanResume(*_ridesTarget))
                        result.insert(result.begin(), "Place entrance / exit...");
                    if (_ridesTarget && Handheld::RideAccess::Supported(*_ridesTarget))
                    {
                        result.insert(
                            result.begin() + (Handheld::FlatRides::CanResume(*_ridesTarget) ? 1 : 0), "Entrances / exits...");
                        result.push_back("Access / paths...");
                    }
                    if (_ridesTarget && Handheld::CustomTracks::CanResume(*_ridesTarget))
                        result.push_back("Continue track...");
                    if (_ridesTarget && Handheld::Mazes::CanResume(*_ridesTarget))
                        result.push_back("Edit maze...");
                    if (_ridesTarget && Handheld::TrackEditing::Eligible(*_ridesTarget))
                        result.push_back("Edit track...");
                    return result;
                }
                case Page::customChoice:
                    return { "Custom build", "Saved designs...", "Back" };
                case Page::build:
                    return { "Raise hill",
                             "Dig",
                             "Paths",
                             "Pond",
                             "Surface",
                             "Remove scenery",
                             "Trees",
                             "Shops / Facilities",
                             "Rides",
                             "Saved designs",
                             "Scenery...",
                             "Terrain / Water...",
                             "Land / Rights...",
                             "Back to park" };
                case Page::sceneryCategory:
                    return Handheld::SceneryTools::Categories();
                case Page::peepOptions:
                    return { "Resume placement", "Cancel placement...", "Back to person" };
                case Page::peepCancel:
                    return Handheld::PeepPlacement::CancelPresentation(_focus.index()).rows;
                case Page::staffPatrolOptions:
                    return Handheld::StaffPatrol::Options();
                case Page::staffPatrolClear:
                    return { "Clear patrol", "Cancel" };
                case Page::rightsOptions:
                    return Handheld::RightsTools::Options();
                case Page::rightsConfirm:
                    return { "Purchase available tiles", "Cancel" };
                case Page::areaOptions:
                    return Handheld::AreaTools::Options();
                case Page::sceneryName:
                case Page::scenerySearch:
                    return _nameEntry.Keys();
                case Page::sceneryConfirm:
                    return { "Apply text", "Cancel" };
                case Page::sceneryClearOptions:
                    return Handheld::SceneryClear::Options();
                case Page::terrainPaintConfirm:
                    return { "Apply paint", "Cancel" };
                case Page::sceneryClearConfirm:
                    return { "Clear area", "Cancel" };
                case Page::sceneryGroups:
                {
                    std::vector<std::string> list;
                    for (const auto& group : Handheld::SceneryTools::Groups(Handheld::SceneryTools::Current().category))
                        list.push_back(group.name);
                    return list;
                }
                case Page::sceneryStyle:
                {
                    std::vector<std::string> result;
                    for (auto style : signStyles())
                        result.push_back(
                            style == Handheld::SceneryTools::Style::noEntry          ? "No entry..."
                                : style == Handheld::SceneryTools::Style::textColour ? "Text colour..."
                                                                                     : "Primary colour...");
                    result.push_back("Back");
                    return result;
                }
                case Page::sceneryStyleEditor:
                    return { _signStyle ? Handheld::SceneryTools::StyleValue(*_signStyle) : "Sign unavailable", "Review change",
                             "Cancel" };
                case Page::sceneryStyleConfirm:
                    return { "Apply style", "Cancel" };
                case Page::sceneryOptions:
                    return Handheld::SceneryTools::Options();
                case Page::sceneryPicker:
                    return {};
                case Page::designOptions:
                    return { "Place in park",
                             Handheld::Designs::HeightNotice(),
                             std::string("Automatic height: ") + (Handheld::Designs::HeightOffset() ? "Off" : "On"),
                             "Facing: "
                                 + std::string(
                                     std::array{ "North", "East", "South", "West" }[Handheld::Designs::Orientation() & 3]),
                             std::string("Include scenery: ") + (Handheld::Designs::IncludesScenery() ? "On" : "Off"),
                             "Vehicle...",
                             "Details / warnings..." };
                case Page::designConfirm:
                    return { "Cancel", "Place design" };
                case Page::terrainMaterialPicker:
                case Page::pathSurfacePicker:
                case Page::trackPiecePicker:
                case Page::designPicker:
                case Page::treePicker:
                case Page::shopPicker:
                case Page::ridePicker:
                    return {};
                case Page::paths:
                    return { "Normal path", "Queue path", "Remove path", "Back to build", "Connected path", "Connected queue" };
                case Page::pathOptions:
                    if (Handheld::Paths::IsConnected())
                        return { "Resume",
                                 "Turn next section",
                                 "Climb",
                                 "Descend",
                                 "Continue level",
                                 "Choose another start",
                                 "Choose surface...",
                                 "Undo last section..." };
                    return { "Resume",          "Follow ground", "Raise path",       "Lower path",
                             "Level / sloping", "Turn slope",    "Choose surface..." };
                case Page::pathUndoConfirm:
                    return { "Remove last new section", "Cancel" };
                case Page::parkInspect:
                    return { "Load this park", "Back to files" };
                case Page::designFiles:
                    return _files.Rows();
                case Page::designVehicles:
                {
                    std::vector<std::string> result;
                    for (const auto& d : _designVehicles)
                    {
                        const auto* object = GetContext()->GetObjectManager().GetLoadedObject(d);
                        result.push_back(object ? object->GetName() : std::string("Vehicle unavailable"));
                    }
                    result.push_back("Cancel / back");
                    return result;
                }
                case Page::designWarnings:
                {
                    if (!_designTechnical)
                        return { "Technical details...", "Back to options" };
                    auto result = _designDetailsFromFile ? std::vector<std::string>{} : Handheld::Designs::WarningLines();
                    if (const auto* td = _designDetailsFromFile ? _designMetadata.get() : Handheld::Designs::Draft())
                    {
                        auto detail = Handheld::DesignAvailability::Lines(*td, _designTechnical);
                        result.insert(result.end(), detail.begin(), detail.end());
                    }
                    result.push_back(_designTechnical ? "Hide technical details" : "Technical details...");
                    result.push_back("Back to options");
                    return result;
                }
                case Page::designInspect:
                {
                    std::vector<std::string> result{ fileDesignEligible() ? "Place this design" : "Placement unavailable",
                                                     "Back to files", "Choose vehicle" };
                    result.push_back("Details / compatibility...");
                    return result;
                }
                case Page::parkFiles:
                    return _files.Rows();
                case Page::saveFiles:
                {
                    auto result = _files.Rows();
                    result.insert(result.begin(), "Save in this folder...");
                    return result;
                }
                case Page::saveName:
                    return _nameEntry.Keys();
                case Page::saveReview:
                    return { _saveDraft && _saveDraft->baseline ? "Replace named park" : "Save named park", "Cancel" };
                case Page::save:
                case Page::load:
                {
                    std::vector<std::string> result;
                    for (size_t slot = 0; slot < 3; ++slot)
                    {
                        std::error_code error;
                        const bool exists = fs::exists(SlotPath(slot), error);
                        result.push_back(
                            "Slot " + std::to_string(slot + 1) + " / "
                            + (_slotPreviews[slot] ? Handheld::SavedParkPresentation::Date(*_slotPreviews[slot])
                                   : exists        ? "Saved park"
                                                   : "Empty"));
                    }
                    result.push_back(_page == Page::load ? "Browse parks / autosaves..." : "Named Save As...");
                    return result;
                }
                case Page::overwrite:
                    return { "Replace this saved park", "Cancel" };
                case Page::discard:
                    return { "Load and discard current changes", "Cancel" };
                case Page::exit:
                    return { "Quit without saving", "Cancel" };
            }
            return {};
        }
        void save()
        {
            const auto draft = _saveDraft;
            const auto path = _namedSave && draft ? draft->destination : SlotPath(_slot);
            if (_namedSave && (!draft || !draft->Current()))
            {
                _saveDraft.reset();
                _status = "Destination changed. Cancel and review again.";
                invalidate();
                return;
            }
            const bool saved = Handheld::WriteSaveAtomically(
                path,
                [](const fs::path& temporary) {
                    SaveFlags saveFlags{};
                    saveFlags.set(SaveFlag::exportObjects, Config::Get().general.savePluginData);
                    return ScenarioSave(getGameState(), temporary.u8string(), saveFlags) != 0;
                },
                [&] { return !draft || draft->Current(); });
            if (!saved)
            {
                _status = "Save failed. Your previous save is kept.";
                invalidate();
                return;
            }
            gScenarioSavePath = path.u8string();
            gCurrentLoadedPath = path.u8string();
            gFirstTimeSaving = false;
            gIsAutosaveLoaded = false;
            if (_exitAfterSave)
            {
                close();
                // Saving succeeded and Pocket already owns the exit decision.
                // Quit() would reopen the desktop save prompt; Finish() lets the
                // native game loop return through its normal cleanup instead.
                GetContext()->Finish();
                return;
            }
            page(Page::park);
            _status = "Saved to " + path.filename().u8string();
            _saveDraft.reset();
        }
        void load()
        {
            if (!_loadFile || !Handheld::FileBrowsing::Current(*_loadFile))
            {
                _status = "File removed, changed or unreadable. Select it again.";
                invalidate();
                return;
            }
            const auto path = _loadFile->path.u8string();
            const bool browsing = _fileLoad;
            const auto folder = _files.directory;
            const auto pattern = _files.pattern;
            Handheld::GuestFollow::Stop();
            Handheld::EndTerrain();
            close();
            if (!GetContext()->LoadParkFromFile(path))
            {
                // Import may close all windows or change scenes. Never access this after close/load.
                auto* recovery = OpenPanel();
                if (recovery)
                    recovery->recoverLoad(folder, pattern, browsing, fs::u8path(path));
            }
        }

    public:
        void recoverLoad(const fs::path& folder, const std::string& pattern, bool browsing, const fs::path& selected)
        {
            _fileLoad = browsing;
            if (browsing)
            {
                _files.Open(folder, pattern);
                _fileFocus = 0;
                for (size_t i = 0; i < _files.entries.size(); ++i)
                    if (_files.entries[i].path == selected)
                        _fileFocus = i + Handheld::FileBrowsing::kFirst;
                page(Page::parkFiles);
            }
            else
                page(Page::load);
            _status = "Could not load park. Choose another file or Back.";
            invalidate();
        }
        size_t dashboardSectionIndex() const
        {
            return _dashboard.index();
        }
        bool overviewIsOpen() const
        {
            return overviewManagement();
        }
        Handheld::ManagementPresentation overviewModel() const
        {
            return overviewPresentation();
        }
        bool ridesIsOpen() const
        {
            return ridesManagement();
        }
        Handheld::ManagementPresentation ridesPresentation() const
        {
            return ridesPresentationFor(_page, _focus.index());
        }
        bool guestsIsOpen() const
        {
            return guestManagement();
        }
        Handheld::ManagementPresentation guestsPresentation() const
        {
            return guestPresentationFor(_page, _focus.index());
        }
        void onOpen() override
        {
            Handheld::StaffPatrol::HideOverlay();
            Drawing::pickupPeepClear();
            Handheld::Paths::ClearPreview();
            Handheld::Trees::ClearPreview();
            Handheld::SceneryTools::ClearPreview();
            Handheld::Shops::ClearPreview();
            Handheld::FlatRides::ClearPreview();
            Handheld::Mazes::ClearPreview();
            Handheld::Designs::ClearPreview();
            Handheld::CustomTracks::ClearPreview();
            setWidgets(kWidgets);
            layout();
            colours[0] = ColourWithFlags{ Colour::grey };
            colours[1] = ColourWithFlags{ Colour::grey };
            _pausedHere = gLegacyScene == LegacyScene::playing && GameIsNotPaused();
            if (_pausedHere)
                PauseToggle();
        }
        void onClose() override
        {
            Handheld::DesignAuthoring::End();
            if (_ridesReviewResumed || _rightsReviewResumed || _clearReviewResumed || _paintReviewResumed || _pathUndoResumed)
                _pausedHere = false; // Our pause was already released; retain any external pause.
            if (_page == Page::designConfirm && _focus.index() == 0)
                Handheld::Designs::CancelReview();
            if (_pausedHere && gLegacyScene == LegacyScene::playing && (gGamePaused & GAME_PAUSED_NORMAL))
                PauseToggle();
        }
        void resumePark()
        {
            if (gLegacyScene == LegacyScene::playing && (gGamePaused & GAME_PAUSED_NORMAL))
                PauseToggle();
            _pausedHere = false;
            close();
        }
        void openQuit(bool standalone)
        {
            if (_page == Page::quitChoice || _page == Page::exit || _quitSaving)
                return;
            if (guardLeave([this, standalone] { openQuit(standalone); }))
                return;
            _quitStandalone = standalone;
            _quitCaller = _page;
            _quitFocus = _focus.index();
            page(Page::quitChoice);
            _focus = {};
            _focus.move(2, 3);
        }
        bool canConnectAccessPoint() const
        {
            if (!_accessPoint || !Handheld::RideAccess::Current(*_accessPoint))
                return false;
            const auto state = Handheld::RideAccess::Inspect(_accessPoint->target);
            // The existing guided handoff chooses Inspect's next unresolved station.
            // Only offer it here when that is the point actually being inspected.
            return state && (_accessPoint->exit ? state->exit.station : state->entrance.station) == _accessPoint->station;
        }
        void openAccessManagement()
        {
            if (!_ridesTarget)
                return;
            _accessPoints = Handheld::RideAccess::Existing(*_ridesTarget);
            _dashboard.section = Handheld::Dashboard::Section::rides;
            _dashboard.focus = Handheld::Dashboard::Focus::content;
            page(Page::accessManage);
            _focus.move(static_cast<int32_t>(std::min(_accessFocus, _accessPoints.size())), rows().size());
        }
        void openAccessReview(const Handheld::RideAccess::EditDraft& draft)
        {
            _ridesTarget = draft.point.target;
            _accessPoint = draft.point;
            _accessDraft.emplace(draft);
            _dashboard.section = Handheld::Dashboard::Section::rides;
            _dashboard.focus = Handheld::Dashboard::Focus::content;
            page(Page::accessReview);
        }
        void cancelAccessReview()
        {
            if (!_accessDraft)
            {
                openAccessManagement();
                return;
            }
            const auto draft = *_accessDraft;
            _accessDraft.reset();
            if (Handheld::FlatRides::CanResume(draft.point.target))
            {
                Handheld::EndTerrain();
                openRideAccess(draft.point.target);
            }
            else if (draft.remove)
                page(Page::accessPoint);
            else
                close(); // Existing relocation cursor remains; no native change on Cancel.
        }
        void openRideAccess(const Handheld::RideManagement::Target& target)
        {
            _ridesTarget = target;
            _dashboard.section = Handheld::Dashboard::Section::rides;
            _dashboard.focus = Handheld::Dashboard::Focus::content;
            page(Page::ridesAccess);
        }
        void openBuild()
        {
            page(Page::build);
        }
        void openPathOptions()
        {
            page(Page::pathOptions);
        }
        bool _terrainMaterialEdge{}, _terrainMaterialFromSettings{}, _sceneryPickerDirect{}, _pathSurfaceDirect{};
        void openTerrainMaterialPicker(bool edge, bool fromSettings = false)
        {
            _terrainMaterialFromSettings = fromSettings;
            _terrainMaterialEdge = edge;
            page(Page::terrainMaterialPicker);
            const auto entries = Handheld::TerrainMaterials::Entries(edge);
            const auto index = edge ? Handheld::AreaTools::settings.edge : Handheld::AreaTools::settings.surface;
            const auto it = std::find_if(
                entries.begin(), entries.end(), [&](const auto& e) { return e.identity.index == index; });
            _picker.setEntries(entries, it == entries.end() ? std::nullopt : std::optional{ it->identity });
        }
        void openSceneryCatalog()
        {
            page(Page::sceneryPicker);
            _sceneryPickerDirect = true;
            refreshPicker();
        }
        void openPathSurfacePicker(bool direct = false)
        {
            _pathSurfaceDirect = direct;
            Handheld::Paths::ClearPreview();
            _pathOptionsFocus[Handheld::Paths::IsConnected()] = 6;
            page(Page::pathSurfacePicker);
            _picker.setEntries(Handheld::PathSurfaces::Entries(), Handheld::PathSurfaces::Current());
        }
        void refreshPicker(std::optional<Handheld::PickerIdentity> preferred = {})
        {
            auto rideEntries = _page == Page::ridePicker ? rideCatalog() : std::vector<Handheld::PickerEntry>{};
            _picker.setEntries(
                _page == Page::terrainMaterialPicker   ? Handheld::TerrainMaterials::Entries(_terrainMaterialEdge)
                    : _page == Page::pathSurfacePicker ? Handheld::PathSurfaces::Entries()
                    : _page == Page::trackPiecePicker  ? Handheld::CustomTracks::Pieces(true)
                    : _page == Page::designPicker      ? Handheld::Designs::Entries()
                    : _page == Page::ridePicker        ? rideEntries
                    : _page == Page::shopPicker        ? Handheld::Shops::Entries()
                    : _page == Page::sceneryPicker ? Handheld::SceneryTools::Entries(Handheld::SceneryTools::Current().category)
                                                   : Handheld::TreePickerEntries(),
                preferred);
        }
        void openRightsOptions()
        {
            page(Page::rightsOptions);
        }
        void openRightsReview(CoordsXY tile)
        {
            if (_pausedHere && (gGamePaused & GAME_PAUSED_NORMAL))
            {
                PauseToggle();
                _rightsReviewResumed = true;
            }

            const auto q = Handheld::RightsTools::Query(tile, Handheld::RightsTools::settings);
            _rightsDraft = Handheld::RightsTools::Begin(tile, Handheld::RightsTools::settings);
            if (_rightsDraft)
            {
                page(Page::rightsConfirm);
                _focus.move(1, 2);
            }
            else
            {
                page(Page::rightsOptions);
                _status = q.getErrorMessage();
            }
        }
        void openAreaOptions()
        {
            page(Page::areaOptions);
        }
        void openTerrainPaintReview(CoordsXY tile)
        {
            if (_pausedHere && (gGamePaused & GAME_PAUSED_NORMAL))
            {
                PauseToggle();
                _paintReviewResumed = true;
            }
            _paintDraft = Handheld::AreaTools::BeginPaint(tile, Handheld::AreaTools::settings);
            if (_paintDraft)
            {
                page(Page::terrainPaintConfirm);
                _focus.move(1, 2);
            }
            else
            {
                page(Page::areaOptions);
                _status = "Paint unavailable; choose a material and wait for pending changes";
            }
        }
        void openSceneryClearOptions()
        {
            page(Page::sceneryClearOptions);
        }
        void openSceneryClearReview(CoordsXY tile)
        {
            if (_pausedHere && (gGamePaused & GAME_PAUSED_NORMAL))
            {
                PauseToggle();
                _clearReviewResumed = true;
            }
            const auto query = Handheld::SceneryClear::Query(tile, Handheld::SceneryClear::settings);
            _clearDraft = Handheld::SceneryClear::Begin(tile, Handheld::SceneryClear::settings);
            if (_clearDraft)
            {
                page(Page::sceneryClearConfirm);
                _focus.move(1, 2);
            }
            else
            {
                page(Page::sceneryClearOptions);
                _status = query.getErrorMessage();
            }
        }
        void openSceneryStyle(const Handheld::SceneryTools::Target& target)
        {
            _signTarget = target;
            page(Page::sceneryStyle);
        }
        void openSceneryOptions()
        {
            page(Page::sceneryOptions);
        }
        bool sceneryPicker() const
        {
            return _page == Page::sceneryPicker;
        }
        void openScenerySearch()
        {
            _scenerySearchParent = _page == Page::sceneryPicker ? Page::sceneryPicker : Page::sceneryOptions;
            _scenerySearchRow = _focus.index();
            _nameEntry = { Handheld::SceneryTools::Search() };
            page(Page::scenerySearch);
        }
        void openTreePicker()
        {
            page(Page::treePicker);
            refreshPicker(Handheld::PickerIdentity{ ObjectType::smallScenery, Handheld::TreeSelection() });
        }
        void openShopPicker(bool fromBuild = false)
        {
            Handheld::Shops::ClearPreview();
            Handheld::FlatRides::ClearPreview();
            Handheld::Mazes::ClearPreview();
            Handheld::Designs::ClearPreview();
            Handheld::CustomTracks::ClearPreview();
            _shopPickerFromBuild = fromBuild;
            page(Page::shopPicker);
            refreshPicker(Handheld::PickerIdentity{ ObjectType::ride, Handheld::ShopSelection() });
        }
        void openRidePicker(bool fromBuild = false)
        {
            Handheld::FlatRides::ClearPreview();
            Handheld::Mazes::ClearPreview();
            Handheld::Designs::ClearPreview();
            Handheld::CustomTracks::ClearPreview();
            _ridePickerFromBuild = fromBuild;
            page(Page::ridePicker);
            refreshPicker(Handheld::PickerIdentity{ ObjectType::ride, Handheld::FlatRides::Selection() });
        }
        bool designPickerOpen() const
        {
            return _page == Page::designPicker;
        }
        void openDesignFiles(bool fromBuild = false)
        {
            TrackRepositoryScan();
            Handheld::Designs::ClearPreview();
            Handheld::Designs::CancelReview();
            _designPickerFromBuild = fromBuild;
            auto folder = fs::u8path(Ui::FileBrowser::GetInitialDirectoryByType(LoadSaveType::track));
            if (!Handheld::Designs::Selection().empty())
                folder = fs::u8path(Handheld::Designs::Selection()).parent_path();
            _files.Open(folder, Ui::FileBrowser::GetFilterPatternByType(LoadSaveType::track, false));
            _fileFocus = 0;
            _fileDetail = 0;
            for (size_t i = 0; i < _files.entries.size(); ++i)
                if (_files.entries[i].path.u8string() == Handheld::Designs::Selection())
                    _fileFocus = i + Handheld::FileBrowsing::kFirst;
            page(Page::designFiles);
        }
        void openDesignPicker(bool fromBuild = false)
        {
            if (Handheld::Designs::BrowserReturn())
            {
                openDesignFiles(fromBuild);
                return;
            }
            TrackRepositoryScan();
            Handheld::Designs::ClearPreview();
            Handheld::CustomTracks::ClearPreview();
            Handheld::Designs::CancelReview();
            _designPickerFromBuild = fromBuild;
            page(Page::designPicker);
            refreshPicker(Handheld::PickerIdentity{ ObjectType::ride, kObjectEntryIndexNull, Handheld::Designs::Selection() });
        }
        void pauseDesignReview()
        {
            Handheld::Designs::CancelReview();
            page(Page::park);
        }
        void openDesignOptions()
        {
            page(Page::designOptions);
            refreshDesignWorkspace();
        }
        void openDesignReview()
        {
            page(Page::designConfirm);
            _status = "Cost " + Handheld::FormatMoney(Handheld::Designs::ReviewCost()) + " / "
                + (Handheld::Designs::HeightOffset() ? Handheld::Designs::HeightNotice() : Handheld::Designs::SceneryNotice());
        }
        bool pickerCamera()
        {
            if (_page != Page::trackPiecePicker || !Handheld::CustomTracks::Active())
                return false;
            close();
            Handheld::CustomTracks::ToggleCamera();
            return true;
        }
        void openTrackPiecePicker()
        {
            Handheld::CustomTracks::ClearPreview();
            page(Page::trackPiecePicker);
            refreshPicker(
                Handheld::PickerIdentity{ ObjectType::ride, kObjectEntryIndexNull,
                                          std::to_string(static_cast<int>(Handheld::CustomTracks::Selection())) });
        }
        void openNews(bool fromPark)
        {
            _newsDashboard = _dashboard;
            _newsFromPark = fromPark;
            _newsFromDashboard = !fromPark;
            _parkNews = Handheld::ParkInformation::NewsList();
            _parkNewsFocus = 0;
            _parkNewsPage = 0;
            Handheld::Alerts::Invalidate();
            _parkAlerts = Handheld::Alerts::Current();
            _dashboard.section = Handheld::Dashboard::Section::overview;
            _dashboard.focus = Handheld::Dashboard::Focus::content;
            if (fromPark && !_parkNews.empty())
            {
                _parkNewsTarget = _parkNews.front();
                page(Page::parkNewsDetail);
                if (Handheld::ParkInformation::SubjectCurrent(*_parkNewsTarget)
                    || Handheld::ManagementNews::Available(*_parkNewsTarget) != Handheld::ManagementNews::Link::none)
                    _focus.move(0, rows().size());
            }
            else
                page(Page::parkNews);
        }
        // Hold Y in the park: the inbox list itself, alerts first, rather than the latest item.
        void openInbox()
        {
            openNews(false);
            _newsFromDashboard = false;
            _newsFromPark = true;
        }
        // Scenario start shows the objective as a Pocket page instead of the desktop window;
        // B from it returns straight to the park, matching news opened from the park.
        // The first-run controls intro. From the guide it returns there; otherwise it closes
        // into the park. Finishing or skipping it marks it seen.
        bool _primerFromGuide{};
        void openPrimer(bool fromGuide)
        {
            _primerFromGuide = fromGuide;
            page(Page::primer);
            _focus = {};
            _focus.move(0, rows().size());
        }
        void finishPrimer()
        {
            auto& store = Handheld::MenuThemes::settings;
            store.Load(GetContext()->GetPlatformEnvironment().GetDirectoryPath(DirBase::user));
            if (!Handheld::preferences.primerSeen)
            {
                Handheld::preferences.primerSeen = true;
                store.SavePreferences();
            }
            if (_primerFromGuide)
            {
                page(Page::controls);
                _focus = {};
                _focus.move(3, rows().size());
            }
            else
                close();
        }
        bool controlsIntro()
        {
            if (_page != Page::controls)
                return false;
            openPrimer(true);
            return true;
        }
        void openObjectives()
        {
            _parkTabs.tab = 1;
            _focus = {};
            page(Page::overview);
            _objectivesFromPark = true;
        }
        bool dashboardNews()
        {
            if (_page != Page::dashboard)
                return false;
            openNews(false);
            return true;
        }
        bool pickerGroup()
        {
            if (_page == Page::ridePicker)
            {
                _rideCategory = (_rideCategory + 1) % kRideCategories.size();
                _picker = {};
                refreshPicker();
                _status.clear();
                invalidate();
                return true;
            }
            if (_page == Page::researchCatalog)
            {
                _researchFilter = (_researchFilter + 1) % 8;
                _researchCatalog = Handheld::ResearchCatalog::Roster(_researchFilter);
                _focus = {};
                invalidate();
                return true;
            }
            if (_page != Page::trackPiecePicker || !Handheld::CustomTracks::Active())
                return false;
            Handheld::CustomTracks::NextGroup();
            _picker = {};
            refreshPicker();
            _status.clear();
            invalidate();
            return true;
        }
        bool picker() const
        {
            return _page == Page::terrainMaterialPicker || _page == Page::pathSurfacePicker || _page == Page::sceneryPicker
                || _page == Page::trackPiecePicker || _page == Page::designPicker || _page == Page::treePicker
                || _page == Page::shopPicker || _page == Page::ridePicker;
        }
        bool treePicker() const
        {
            return _page == Page::treePicker;
        }
        ObjectEntryIndex pickerFocus() const
        {
            return _picker.selected() ? _picker.selected()->identity.index : kObjectEntryIndexNull;
        }
        void pickerMove(int x, int y)
        {
            if (!picker() || (x == 0 && y == 0))
                return;
            // In the ride catalog the shoulders move through the type tabs instead of paging.
            if (_page == Page::ridePicker && (y == 2 || y == -2))
            {
                const auto count = kRideCategories.size();
                _rideCategory = (_rideCategory + (y > 0 ? 1 : count - 1)) % count;
                _picker = {};
                refreshPicker();
                _status.clear();
                invalidate();
                return;
            }
            refreshPicker();
            _picker.move(x, y);
            _status.clear();
            invalidate();
        }
        // Sounds follow what changed on screen, so every input path (lists, grids, rails,
        // tabs) gets the same feedback without each handler playing its own.
        void playFeelSound(Handheld::MenuFeel::Event event)
        {
            Handheld::PlayFeelSound(event, _feelBack);
            _feelBack = false;
        }
        void pickerPause()
        {
            if (picker())
                page(Page::park); // Discard unconfirmed focus, keep the placement selection.
        }
        void onUpdate() override
        {
            playFeelSound(_menuFeel.Observe(
                scrollKey(), _focus.index() + (_dashboard.index() << 16), Handheld::MenuFeel::Clock(),
                static_cast<int>(_page)));
            if (_menuFeel.NeedsDraw())
                invalidate();
            if (managementShell() && Handheld::MenuFeel::Clock() - _previewRefreshed > 500)
            {
                _previewRefreshed = Handheld::MenuFeel::Clock();
                invalidate();
            }
            if (_scrollViews[scrollKey()].moving())
                invalidate();
            if (_page == Page::sceneryPicker)
            {
                refreshPicker();
                invalidate();
            }
            if (_page == Page::parkMap)
            {
                _parkMap.Update();
                invalidate();
            }
            if (parkInformation())
            {
                _ledger.Clamp();
                invalidate();
            }
            if (_page == Page::researchCatalog)
            {
                const auto updated = Handheld::ResearchCatalog::Roster(_researchFilter);
                if (updated != _researchCatalog)
                {
                    const auto selected = _focus.index() < _researchCatalog.size()
                        ? std::optional<Handheld::ResearchCatalog::Target>{ _researchCatalog[_focus.index()] }
                        : std::nullopt;
                    const auto found = selected ? std::find(updated.begin(), updated.end(), *selected) : updated.end();
                    const auto row = found == updated.end() ? std::min(_focus.index(), updated.size())
                                                            : static_cast<size_t>(found - updated.begin());
                    _researchCatalog = updated;
                    _focus = {};
                    _focus.move(static_cast<int>(row), rows().size());
                }
            }
            if (_page == Page::mapSubjects)
            {
                const auto updated = Handheld::MapSubjects::Roster(_parkMap.mode, _parkMap.filter, _parkMap.guestFilter);
                if (updated != _mapSubjects)
                {
                    const auto selected = _focus.index() < _mapSubjects.size()
                        ? std::optional<Handheld::MapSubjects::Target>{ _mapSubjects[_focus.index()] }
                        : std::nullopt;
                    const auto found = selected ? std::find(updated.begin(), updated.end(), *selected) : updated.end();
                    const auto row = found == updated.end() ? std::min(_focus.index(), updated.size())
                                                            : static_cast<size_t>(found - updated.begin());
                    _mapSubjects = updated;
                    _focus = {};
                    _focus.move(static_cast<int>(row), rows().size());
                }
            }
            if (_page == Page::ridesRoster)
            {
                const auto updated = Handheld::RideManagement::Roster();
                if (updated != _ridesRoster)
                {
                    const auto selected = _focus.index() < _ridesRoster.size()
                        ? std::optional<Handheld::RideManagement::Target>{ _ridesRoster[_focus.index()] }
                        : std::nullopt;
                    const auto found = selected ? std::find(updated.begin(), updated.end(), *selected) : updated.end();
                    const auto row = found == updated.end() ? std::min(_focus.index(), updated.size())
                                                            : static_cast<size_t>(found - updated.begin());
                    _ridesRoster = updated;
                    _ridesRosterFocus = row;
                    _focus = {};
                    _focus.move(static_cast<int32_t>(row), rows().size());
                }
            }
            if (ridesManagement() || overviewManagement())
                invalidate();
            if (_page == Page::guestRideHistory && _guestTarget)
            {
                const auto updated = Handheld::GuestManagement::RideHistory(*_guestTarget);
                if (updated != _guestRides)
                {
                    const auto selected = _focus.index() < _guestRides.size()
                        ? std::optional<Handheld::RideManagement::Target>{ _guestRides[_focus.index()] }
                        : std::nullopt;
                    const auto found = selected ? std::find(updated.begin(), updated.end(), *selected) : updated.end();
                    const auto row = found == updated.end() ? std::min(_focus.index(), updated.size())
                                                            : static_cast<size_t>(found - updated.begin());
                    _guestRides = updated;
                    _guestRideFocus = row;
                    _focus = {};
                    _focus.move(static_cast<int>(row), rows().size());
                }
                invalidate();
            }
            if (_page == Page::guestGroups)
            {
                const auto updated = Handheld::GuestManagement::Groups(_guestGroupMode);
                if (updated != _guestGroups)
                {
                    size_t row = std::min(_focus.index(), updated.size());
                    if (_focus.index() < _guestGroups.size())
                    {
                        const auto key = _guestGroups[_focus.index()].arguments;
                        const auto found = std::find_if(
                            updated.begin(), updated.end(), [&](const auto& g) { return g.arguments == key; });
                        if (found != updated.end())
                            row = std::distance(updated.begin(), found);
                    }
                    _guestGroups = updated;
                    _focus = {};
                    _focus.move(static_cast<int>(row), rows().size());
                    invalidate();
                }
            }
            if (_page == Page::guestRoster)
            {
                const auto updated = Handheld::GuestManagement::Roster(_guestFilter);
                if (updated != _guestRoster)
                {
                    const auto selected = _focus.index() >= 3 && _focus.index() - 3 < _guestRoster.size()
                        ? std::optional<Handheld::GuestManagement::Target>{ _guestRoster[_focus.index() - 3] }
                        : std::nullopt;
                    size_t row = std::min(_focus.index(), updated.size() + 3);
                    if (selected)
                        if (const auto found = std::find(updated.begin(), updated.end(), *selected); found != updated.end())
                            row = 3 + std::distance(updated.begin(), found);
                    _guestRoster = updated;
                    _guestRosterFocus = row;
                    _focus = {};
                    _focus.move(static_cast<int32_t>(row), rows().size());
                    invalidate();
                }
            }
            if (financeManagement() || researchManagement() || marketingManagement())
                invalidate();
            if (_page == Page::marketing && _marketingTabs.tab == 0)
            {
                const auto updated = Handheld::MarketingManagement::ActiveTypes();
                if (updated != _marketingActiveTypes)
                {
                    const auto selected = _focus.index() < _marketingActiveTypes.size() ? _marketingActiveTypes[_focus.index()]
                                                                                        : -1;
                    const auto found = std::find(updated.begin(), updated.end(), selected);
                    _marketingActiveTypes = updated;
                    _focus = {};
                    _focus.move(found == updated.end() ? 0 : static_cast<int>(found - updated.begin()), rows().size());
                }
            }
            if (guestManagement() || staffManagement())
                invalidate(); // read native state without clearing native guest invalidation flags
            if (_page == Page::staff)
            {
                const auto updated = Handheld::StaffManagement::Roster(_staffFilter);
                if (updated != _staffRoster)
                {
                    std::optional<Handheld::StaffManagement::Target> selected;
                    if (_focus.index() >= 3 && _focus.index() - 3 < _staffRoster.size())
                        selected = _staffRoster[_focus.index() - 3];
                    size_t newFocus = std::min(_focus.index(), updated.size() + 3);
                    if (selected)
                    {
                        const auto found = std::find(updated.begin(), updated.end(), *selected);
                        if (found != updated.end())
                            newFocus = 3 + std::distance(updated.begin(), found);
                    }
                    _staffRoster = updated;
                    _focus = {};
                    _focus.move(static_cast<int32_t>(newFocus), rows().size());
                    invalidate();
                }
            }
            if (shopManagement() && _shop && !Handheld::ShopManagement::Find(*_shop))
            {
                _shop.reset();
                _status = "Shop no longer available";
                invalidate();
            }
            if ((treePicker() && !Handheld::IsTreeToolActive())
                || (_page == Page::shopPicker && !_shopPickerFromBuild && !Handheld::IsShopToolActive())
                || (_page == Page::ridePicker && !_ridePickerFromBuild
                    && Handheld::FlatRides::CurrentStage() != Handheld::FlatRides::Stage::ride))
                close();
        }
        Handheld::RideNavigation::Context rideContext() const
        {
            return { *_ridesTarget, _rideTabs, _rideFromPark, _ridesRosterFocus };
        }
        template<typename Begin>
        void launchRideTool(Begin begin)
        {
            const auto context = rideContext();
            close();
            if (begin())
                Handheld::RideNavigation::construction = context;
            else
                HandheldRideReturn(context.target, context);
        }
        bool quickRideAccess()
        {
            if (_page != Page::ridesInspect || !_ridesTarget || !Handheld::RideAccess::Supported(*_ridesTarget))
                return false;
            _rideTabs.save(_focus.index());
            if (Handheld::FlatRides::CanResume(*_ridesTarget))
                openRideAccess(*_ridesTarget);
            else
                openAccessManagement();
            return true;
        }
        void returnRide(
            const Handheld::RideManagement::Target& target, const std::optional<Handheld::RideNavigation::Context>& context)
        {
            if (!Handheld::RideManagement::Resolve(target))
            {
                close();
                return;
            }
            openRide(target.id);
            // What the tool just did (auto-placed access, say), shown until the next page change.
            if (!sRideReturnNotice.empty())
            {
                _status = std::exchange(sRideReturnNotice, {});
                invalidate();
            }
            if (context && context->target == target)
            {
                _rideTabs = context->tabs;
                _rideFromPark = context->fromPark;
                _ridesRosterFocus = context->rosterFocus;
                _focus = {};
                _focus.move(static_cast<int>(_rideTabs.rows[_rideTabs.tab]), rows().size());
                invalidate();
            }
        }
        void openRide(RideId id)
        {
            _rideId = id;
            _ridesInspectFocus = 0;
            _rideTabs = {};
            _ridesTabChild = false;
            _ridesTarget = Handheld::RideManagement::Identify(id);
            _shop = Handheld::ShopManagement::Capture(id);
            const auto* ride = GetRide(id);
            const bool isShop = ride && ride->getRideTypeDescriptor().flags.has(RtdFlag::isShopOrFacility);
            if (isShop && _pausedHere && GameIsPaused())
            {
                // Keep a shop live; native demolition must obey actual pause rules.
                PauseToggle();
                _pausedHere = false;
            }
            _rideFromPark = true;
            if (isShop)
                page(Page::shop);
            else
            {
                _dashboard.section = Handheld::Dashboard::Section::rides;
                _dashboard.focus = Handheld::Dashboard::Focus::content;
                page(Page::ridesInspect);
                const auto visible = rows();
                // Start on the status change that matters (Close for an open ride, Open otherwise) rather
                // than offering Open on a ride that is already running. Both lead to a confirmation.
                const bool running = ride && ride->status == RideStatus::open;
                const auto primary = std::find(visible.begin(), visible.end(), running ? "Close..." : "Open...");
                if (primary != visible.end())
                {
                    _focus = {};
                    _focus.move(static_cast<int>(primary - visible.begin()), visible.size());
                }
                // Incomplete native access remains a one-press contextual action.
                const auto place = std::find(visible.begin(), visible.end(), "Place entrance / exit...");
                if (place != visible.end())
                {
                    _focus = {};
                    _focus.move(static_cast<int>(place - visible.begin()), visible.size());
                }
            }
        }
        void openLoad()
        {
            page(Page::load);
        }
        void staffRemoved(EntityId id)
        {
            if (_staffTarget && _staffTarget->id == id)
                _staffTarget.reset();
            std::erase_if(_staffRoster, [id](const auto& target) { return target.id == id; });
            if (staffManagement())
            {
                _status = "Employee removed";
                invalidate();
            }
        }
        bool dashboardIsOpen() const
        {
            return managementShell();
        }
        bool dashboardRailFocused() const
        {
            return managementShell() && _dashboard.focus == Handheld::Dashboard::Focus::rail;
        }
        bool dashboardCanSwitch() const
        {
            return _page == Page::dashboard || _page == Page::overview || _page == Page::finances || _page == Page::staff
                || _page == Page::staffInspect || _page == Page::staffStats || _page == Page::research
                || _page == Page::marketing || _page == Page::marketingActive
                || (guestManagement() && _page != Page::guestName && _page != Page::guestFilterName
                    && _page != Page::guestNameConfirm && _page != Page::guestTracking)
                || _page == Page::ridesRoster || _page == Page::ridesInspect || _page == Page::ridesAccess
                || _page == Page::ridesGraphs || _page == Page::ridesStations || _page == Page::ridesCustomers
                || _page == Page::ridesEarnings || _page == Page::ridesStats || _page == Page::ridesVehicles
                || _page == Page::ridesAppearance || _page == Page::ridesMusic || _page == Page::ridesDispatch
                || _page == Page::ridesOperations || _page == Page::ridesMaintenance;
        }
        // Changes a ride's state straight away: opening, testing and closing are free and can be
        // undone with the same switch, so they need no review screen.
        void stepRideState(int direction, bool wrap)
        {
            const auto* ride = _ridesTarget ? Handheld::RideManagement::Resolve(*_ridesTarget) : nullptr;
            if (!ride)
                return;
            std::vector<RideStatus> states{ RideStatus::closed };
            if (ride->supportsStatus(RideStatus::testing))
                states.push_back(RideStatus::testing);
            states.push_back(RideStatus::open);
            const auto found = std::find(states.begin(), states.end(), ride->status);
            const int current = found == states.end() ? 0 : static_cast<int>(found - states.begin());
            int next = current + direction;
            const int size = static_cast<int>(states.size());
            next = wrap ? (next % size + size) % size : std::clamp(next, 0, size - 1);
            if (next == current)
                return;
            if (auto draft = Handheld::RideManagement::Begin(*_ridesTarget, states[next]))
            {
                const auto result = Handheld::RideManagement::Action(*draft, true);
                if (result.error != GameActions::Status::ok)
                    _status = result.getErrorMessage().empty() ? result.getErrorTitle() : result.getErrorMessage();
                else
                    _status.clear();
            }
            invalidate();
        }
        // Ride prices change in place, one native step per press (holding repeats). Each step is the
        // same RideSetPriceAction the editor uses, so limits and park pricing rules still apply.
        void adjustRidePrice(int direction)
        {
            if (!_ridesTarget)
                return;
            auto draft = Handheld::RideManagement::Begin(*_ridesTarget);
            if (!draft)
            {
                _status = "This park does not charge for rides";
                invalidate();
                return;
            }
            Handheld::RideManagement::AdjustPrice(*draft, direction);
            const auto result = Handheld::RideManagement::Action(*draft, true);
            if (result.error != GameActions::Status::ok)
                _status = result.getErrorMessage().empty() ? result.getErrorTitle() : result.getErrorMessage();
            else
                _status.clear();
            invalidate();
        }
        // Rows above a two-column grid that are changed with Left/Right in place (a state switch, a
        // price). Up/Down walks the controls, then the grid keeps its columns.
        void controlsThenGridMove(int x, int y, size_t controls, const std::function<void(size_t, int)>& adjust)
        {
            const auto count = rows().size();
            const auto current = _focus.index();
            size_t next = current;
            if (current < controls)
            {
                if (x)
                {
                    adjust(current, x);
                    ++_valueSteps;
                }
                else if (y < 0 && current > 0)
                    next = current - 1;
                else if (y > 0 && current + 1 < count)
                    next = current + 1;
            }
            else
            {
                const size_t grid = current - controls, cells = count - controls;
                if (y < 0 && grid < 2)
                    next = controls - 1;
                else if (y)
                {
                    const auto candidate = static_cast<int>(grid) + y * 2;
                    if (candidate >= 0 && candidate < static_cast<int>(cells))
                        next = static_cast<size_t>(candidate) + controls;
                    else if (y > 0 && grid / 2 < (cells - 1) / 2)
                        next = count - 1;
                }
                else if (x < 0 && grid % 2)
                    --next;
                else if (x > 0 && grid % 2 == 0 && grid + 1 < cells)
                    ++next;
            }
            _focus.move(static_cast<int>(next) - static_cast<int>(current), count);
            invalidate();
        }
        void stepFunding(int direction, bool wrap)
        {
            if (!Handheld::ResearchManagement::FundingAvailable())
            {
                _status = "Research funding unavailable";
                invalidate();
                return;
            }
            auto draft = Handheld::ResearchManagement::Begin();
            const int next = wrap ? (draft.funding + direction + 4) % 4 : std::clamp<int>(draft.funding + direction, 0, 3);
            if (next == draft.funding)
                return;
            draft.funding = static_cast<uint8_t>(next);
            const auto result = Handheld::ResearchManagement::Action(draft, true);
            _status = result.error == GameActions::Status::ok ? "" : Handheld::Finances::Feedback(result);
            invalidate();
        }
        void setParkOpen(bool open)
        {
            auto draft = Handheld::ParkOverview::Begin();
            if (!draft)
            {
                _status = "Park status locked by objective";
                invalidate();
                return;
            }
            if (draft->baselineOpen == open)
                return;
            const auto result = Handheld::ParkOverview::Action(*draft, true);
            _status = result.error == GameActions::Status::ok ? "" : Handheld::Finances::Feedback(result);
            invalidate();
        }
        void adjustAdmission(int direction)
        {
            if (!Handheld::Finances::Available(Handheld::Finances::Field::admission))
            {
                _status = "Admission price locked";
                invalidate();
                return;
            }
            auto draft = Handheld::Finances::Begin(Handheld::Finances::Field::admission);
            Handheld::Finances::Adjust(draft, direction);
            if (draft.value == draft.baseline)
                return;
            const auto result = Handheld::Finances::Action(draft, true);
            _status = result.error == GameActions::Status::ok ? "" : Handheld::Finances::Feedback(result);
            invalidate();
        }
        void enterDashboardContent()
        {
            using Handheld::Dashboard::Section;
            const auto root = _dashboard.section == Section::staff ? std::optional<Page>{ Page::staff }
                : _dashboard.section == Section::finances          ? std::optional<Page>{ Page::finances }
                : _dashboard.section == Section::research          ? std::optional<Page>{ Page::research }
                : _dashboard.section == Section::marketing         ? std::optional<Page>{ Page::marketing }
                : _dashboard.section == Section::guests            ? std::optional<Page>{ Page::guests }
                : _dashboard.section == Section::rides             ? std::optional<Page>{ Page::ridesRoster }
                : _dashboard.section == Section::overview          ? std::optional<Page>{ Page::overview }
                                                                   : std::nullopt;
            if (!root || !_dashboard.enter())
                return;
            const auto resume = _sectionResume[_dashboard.index()];
            page(resume.page.value_or(*root));
            if (resume.page)
            {
                _focus = {};
                _focus.move(static_cast<int32_t>(std::min(resume.row, rows().size() - 1)), rows().size());
            }
            invalidate();
        }
        void openDashboard()
        {
            page(Page::dashboard);
        }
        // A sign edited straight from the park (context A) goes back to the park, not to the
        // scenery tool's options.
        bool _signFromPark{};
        void leaveSign()
        {
            if (_signFromPark)
            {
                _signFromPark = false;
                close();
            }
            else
                page(Page::sceneryOptions);
        }
        void openSignFromPark()
        {
            _signTarget = Handheld::SceneryEditTarget();
            if (!_signTarget || _signTarget->banner == BannerIndex::GetNull())
            {
                close();
                return;
            }
            _signFromPark = true;
            _nameEntry = { _signTarget->text };
            page(Page::sceneryName);
        }
        // Settings: A or Left/Right flips a row. Theme and Motion keep their own files; the rest
        // live in pocket-prefs.cfg.
        void toggleSetting(size_t row)
        {
            auto& store = Handheld::MenuThemes::settings;
            store.Load(GetContext()->GetPlatformEnvironment().GetDirectoryPath(DirBase::user));
            static constexpr std::array<const char*, 5> keys{ "smartCursor", "hudGoal", "hudClock", "resumeOnLaunch",
                                                              "saveOnSleep" };
            bool saved = true;
            if (row == 0)
                saved = store.Toggle();
            else if (row == 1)
                saved = store.ToggleMotion();
            else if (row >= 2 && row <= 6)
                saved = store.TogglePreference(keys[row - 2]);
            _status = saved ? "" : "Changed for this session; the preference could not be saved";
            invalidate();
        }
        // The quick wheel's destinations. Build reopens on the card used last (Build remembers
        // its focus), and "Again" confirms that card straight away.
        void quickJump(Handheld::QuickWheel::Target target)
        {
            using T = Handheld::QuickWheel::Target;
            using S = Handheld::Dashboard::Section;
            switch (target)
            {
                case T::build:
                    page(Page::build);
                    break;
                case T::repeat:
                    _buildFocus = sLastBuildCard;
                    page(Page::build);
                    if (!sLastBuildLabel.empty())
                        confirm();
                    break;
                case T::map:
                    _parkMap.Begin();
                    page(Page::parkMap);
                    break;
                case T::rides:
                case T::staff:
                case T::finances:
                case T::guests:
                    _dashboard.section = target == T::rides ? S::rides
                        : target == T::staff                ? S::staff
                        : target == T::finances             ? S::finances
                                                            : S::guests;
                    page(Page::dashboard);
                    _dashboard.section = target == T::rides ? S::rides
                        : target == T::staff                ? S::staff
                        : target == T::finances             ? S::finances
                                                            : S::guests;
                    enterDashboardContent();
                    break;
                case T::pause:
                    break;
            }
            invalidate();
        }

        void dashboardPause()
        {
            if (guardLeave([this] { dashboardPause(); }))
                return;
            if (_civicBusy)
                return;
            _managementNewsReturn = false;
            _guestRideInspection = false;
            _guestMapReturn = false;
            _parkMap.guestFilter.reset();
            if (managementShell())
                page(Page::park);
        }
        void dashboardSwitchPane()
        {
            if (guardLeave([this] { dashboardSwitchPane(); }))
                return;
            if (_managementNewsReturn || _guestRideInspection || _guestMapReturn)
                return;
            if (!managementShell() || !dashboardCanSwitch())
                return;
            if (dashboardRailFocused())
                enterDashboardContent();
            else
            {
                if (_page == Page::guestRoster)
                    _guestRosterFocus = _focus.index();
                if (_page == Page::staff)
                    _staffRosterFocus = _focus.index();
                page(Page::dashboard);
            }
        }
        // Left/Right that changed a value (admission, funding, a toggle) rather than moving focus.
        uint32_t _valueSteps{};
        // The rail keeps showing a preview of the focused section; repaint it now and then so the
        // numbers stay live while the game runs.
        uint64_t _previewRefreshed{};
        void dashboardMove(int x, int y)
        {
            // Left that does nothing else in a section's content returns to the section rail, as
            // the rail on the left suggests (B still works too).
            if (x < 0 && !y && !dashboardRailFocused() && !_leaveAction && managementShell() && dashboardCanSwitch())
            {
                const auto page = _page;
                const auto focused = _focus.index();
                const auto steps = _valueSteps;
                dashboardMoveWithin(x, y);
                if (_page == page && _focus.index() == focused && _valueSteps == steps && dashboardCanSwitch())
                {
                    this->page(Page::dashboard);
                    invalidate();
                }
                return;
            }
            dashboardMoveWithin(x, y);
        }
        void dashboardMoveWithin(int x, int y)
        {
            if (_leaveAction)
            {
                if (!_civicBusy)
                    move(y ? y : x);
                return;
            }
            if (dashboardRailFocused())
            {
                if (y)
                {
                    _dashboard.move(y);
                    _status.clear();
                    invalidate();
                }
                else if (x > 0)
                    enterDashboardContent();
            }
            else if (_page == Page::finances && (_financeTabs.tab == 1 || _financeTabs.tab == 2))
            {
                if (x)
                {
                    if (_financeTabs.tab == 1)
                        _ledger.Move(x);
                    else
                        _financeSample = std::clamp<int>(static_cast<int>(_financeSample) - x, 0, 63);
                    ++_valueSteps;
                }
                if (y)
                    move(y);
                _status.clear();
                invalidate();
            }
            else if ((_page == Page::staff || _page == Page::guestRoster) && _focus.index() < 3)
            {
                if (x)
                    _focus.move(
                        std::clamp<int>(static_cast<int>(_focus.index()) + x, 0, 2) - static_cast<int>(_focus.index()),
                        rows().size());
                if (y > 0)
                    move(static_cast<int>(3 - _focus.index()));
                invalidate();
            }
            else if (_page == Page::ridesInspect && _rideTabs.tab == Handheld::RideTabs::overview)
            {
                const auto model = ridesPresentationFor(_page, _focus.index());
                controlsThenGridMove(x, y, model.settingRows.empty() ? 1 : 2, [this](size_t row, int dx) {
                    if (row == 0)
                        stepRideState(dx, false);
                    else
                        adjustRidePrice(dx);
                });
            }
            else if (_page == Page::finances && _financeTabs.tab == 3)
                controlsThenGridMove(x, y, 1, [this](size_t, int dx) { adjustAdmission(dx); });
            else if (_page == Page::research && _researchTabs.tab == 0)
                controlsThenGridMove(x, y, 1, [this](size_t, int dx) { stepFunding(dx, false); });
            else if (_page == Page::overview && !_overviewDraft)
            {
                controlsThenGridMove(x, y, 2, [this](size_t row, int dx) {
                    if (row == 0)
                        setParkOpen(dx > 0);
                    else
                        adjustAdmission(dx);
                });
            }
            else if (
                _page == Page::ridesInspect || _page == Page::overview || _page == Page::finances || _page == Page::research
                || _page == Page::guests || (_page == Page::marketing && _marketingTabs.tab == 1)
                || (_page == Page::staffInspect && _staffTabs.tab != 1)
                || (_page == Page::guestInspect && _guestTabs.tab != 1 && _guestTabs.tab != 2))
            {
                const auto count = rows().size();
                const auto current = _focus.index();
                // A two-column grid: horizontal movement stays in its row;
                // vertical movement preserves its column, including the final odd row.
                size_t next = current;
                if (y)
                {
                    const auto candidate = static_cast<int>(current) + y * 2;
                    if (candidate >= 0 && candidate < static_cast<int>(count))
                        next = static_cast<size_t>(candidate);
                    else if (y > 0 && current / 2 < (count - 1) / 2)
                        next = count - 1; // Enter a final short row without wrapping at the edge.
                }
                else if (x < 0 && current % 2)
                    --next;
                else if (x > 0 && current % 2 == 0 && current + 1 < count)
                    ++next;
                _focus.move(static_cast<int>(next) - static_cast<int>(current), count);
                _status.clear();
                invalidate();
            }
            else if (marketingManagement())
            {
                if (y)
                    move(y);
                if (x && _page == Page::marketingEditor && _marketingDraft
                    && Handheld::MarketingManagement::NeedsTarget(_marketingDraft->type) && _focus.index() == 0)
                {
                    const auto targets = Handheld::MarketingManagement::Targets(_marketingDraft->type);
                    if (!targets.empty())
                    {
                        const auto found = _marketingDraft->target
                            ? std::find(targets.begin(), targets.end(), *_marketingDraft->target)
                            : targets.end();
                        const auto index = found == targets.end()
                            ? (x > 0 ? 0 : static_cast<int>(targets.size()) - 1)
                            : (static_cast<int>(found - targets.begin()) + x + static_cast<int>(targets.size()))
                                % static_cast<int>(targets.size());
                        Handheld::MarketingManagement::SelectTarget(*_marketingDraft, targets[index]);
                    }
                    _status.clear();
                    invalidate();
                }
                if (x && _page == Page::marketingEditor && _marketingDraft
                    && _focus.index() == (Handheld::MarketingManagement::NeedsTarget(_marketingDraft->type) ? 1u : 0u))
                {
                    Handheld::MarketingManagement::AdjustWeeks(*_marketingDraft, x);
                    _status.clear();
                    invalidate();
                }
            }
            else if (researchManagement())
            {
                if (y)
                    move(y);
                if (x && _page == Page::researchEditor && _focus.index() == 0 && _researchDraft)
                {
                    Handheld::ResearchManagement::AdjustFunding(*_researchDraft, x);
                    _status.clear();
                    invalidate();
                }
            }
            else if (_page == Page::ridesGraphs)
            {
                if (y)
                    move(y);
                if (x)
                {
                    const auto* ride = _ridesTarget ? Handheld::RideManagement::Resolve(*_ridesTarget) : nullptr;
                    const auto max = ride && ride->measurement && ride->measurement->num_items
                        ? ride->measurement->num_items - 1
                        : 0;
                    _rideGraphOffset = std::clamp<int>(static_cast<int>(_rideGraphOffset) + x * 120, 0, max);
                    invalidate();
                }
            }
            else if (_page == Page::staffUniform)
            {
                size_t current = 0;
                for (uint8_t i = 0; i < 72; ++i)
                    if (ColourDropDownIndexToColour(i) == _staffUniformDraft)
                        current = i;
                _staffUniformDraft = ColourDropDownIndexToColour(Handheld::MoveColourGrid(current, 72, x, y));
                _status.clear();
                invalidate();
            }
            else if (
                _page == Page::ridesName || _page == Page::designName || _page == Page::staffName || _page == Page::guestName
                || _page == Page::guestFilterName)
            {
                const auto current = _focus.index();
                const auto next = _nameEntry.Move(current, x, y);
                _focus.move(static_cast<int>(next) - static_cast<int>(current), rows().size());
                invalidate();
            }
            else if (_page == Page::ridesPrice)
            {
                if (rideVehicleChoice())
                {
                    auto picker = rideVehiclePicker();
                    picker.move(x, y);
                    _focus.move(static_cast<int>(picker.focus()) - static_cast<int>(_focus.index()), rows().size());
                    _ridesDraft->value = _ridesDraft->setting->values[picker.focus()];
                    invalidate();
                    return;
                }
                if (_ridesDraft && _ridesDraft->setting && _ridesDraft->setting->appearance)
                {
                    const auto next = Handheld::RideManagement::IsColourSetting(*_ridesDraft->setting)
                        ? Handheld::MoveColourGrid(_focus.index(), rows().size(), x, y)
                        : static_cast<size_t>(
                              std::clamp<int>(static_cast<int>(_focus.index()) + (y ? y : x), 0, rows().size() - 1));
                    _focus.move(static_cast<int>(next) - static_cast<int>(_focus.index()), rows().size());
                    _ridesDraft->value = _ridesDraft->setting->values[next];
                    _status.clear();
                    invalidate();
                    return;
                }
                if (y)
                    move(y);
                if (x && _ridesDraft)
                {
                    Handheld::RideManagement::AdjustPrice(*_ridesDraft, x);
                    _status.clear();
                    invalidate();
                }
            }
            else if (financeManagement())
                financeMove(x, y);
            else if (y)
                move(y);
        }
        void dashboardPage(int direction)
        {
            if (_leaveAction)
                return;
            // L1/R1 step the guide's sections and the intro's cards, as their tabs suggest.
            if (_page == Page::controls || _page == Page::primer)
            {
                constructionMove(direction, 0);
                return;
            }
            if (dutiesDirty() && guardLeave([this, direction] { dashboardPage(direction); }))
                return;
            if (dashboardRailFocused())
            {
                _dashboard.move(direction);
                _status.clear();
                invalidate();
                return;
            }
            if (_page == Page::finances)
            {
                financeTab((static_cast<int>(_financeTabs.tab) + direction + 4) % 4);
                return;
            }
            if (_page == Page::research)
            {
                researchTab((static_cast<int>(_researchTabs.tab) + direction + 2) % 2);
                return;
            }
            if (_page == Page::marketing)
            {
                marketingTab((static_cast<int>(_marketingTabs.tab) + direction + 2) % 2);
                return;
            }
            if (_page == Page::overview)
            {
                _parkTabs.save(_focus.index());
                _parkTabs.move(direction);
                _focus = {};
                _focus.move(static_cast<int>(std::min(_parkTabs.rows[_parkTabs.tab], rows().size() - 1)), rows().size());
                _status.clear();
                invalidate();
                return;
            }
            if (_page == Page::staffInspect || _page == Page::guestInspect)
            {
                auto& tabs = _page == Page::staffInspect ? _staffTabs : _guestTabs;
                tabs.save(_focus.index());
                const auto remembered = tabs.move(direction);
                if (_page == Page::staffInspect && _staffTarget)
                    if (const auto* staff = Handheld::StaffManagement::Resolve(*_staffTarget))
                        _ordersBaseline = _ordersDraft = staff->staffOrders;
                _focus = {};
                _focus.move(static_cast<int>(std::min(remembered, rows().size() - 1)), rows().size());
                _status.clear();
                invalidate();
                return;
            }
            if (_page == Page::ridesInspect)
            {
                _rideTabs.save(_focus.index());
                const auto remembered = _rideTabs.move(direction);
                _focus = {};
                _focus.move(static_cast<int>(std::min(remembered, rows().size() - 1)), rows().size());
                _status.clear();
                invalidate();
                return;
            }
            if (!dashboardCanSwitch() || _managementNewsReturn || _guestRideInspection || _guestMapReturn)
                return;
            Handheld::ManagementPresentation model;
            if (ridesManagement())
                model = ridesPresentationFor(_page, _focus.index());
            else if (_page == Page::staff)
                model = staffPresentation();
            else if (_page == Page::guestRoster)
                model = guestsPresentation();
            else
                return;
            const auto workspaceWidth = ContextGetWidth() - 2 * Handheld::Layout::margin;
            const auto capacity = Handheld::MeasureManagement(
                                      { workspaceWidth,
                                        Handheld::Dashboard::ContentHeight(
                                            { ContextGetWidth(), ContextGetHeight() }, _dashboard, model,
                                            dashboardCanSwitch()) },
                                      model)
                                      .capacity;
            const auto current = _focus.index();
            const auto listStart = model.denseRoster ? 3 : 0;
            const auto next = std::clamp<int>(
                std::max(static_cast<int>(current), listStart) + direction * static_cast<int>(capacity), listStart,
                static_cast<int>(rows().size()) - 1);
            _focus.move(next - static_cast<int>(current), rows().size());
            _status.clear();
            invalidate();
        }
        void saveStaffReturn()
        {
            _staffTabs.save(_focus.index());
            Handheld::PeopleNavigation::staff = Handheld::PeopleNavigation::StaffContext{ *_staffTarget, _staffTabs,
                                                                                          _staffRosterFocus, _staffFilter,
                                                                                          _staffFromPark };
        }
        void saveGuestReturn()
        {
            _guestTabs.save(_focus.index());
            Handheld::PeopleNavigation::guest = Handheld::PeopleNavigation::GuestContext{
                *_guestTarget,         _guestTabs,     _guestRosterFocus,
                _guestFilter,          _guestFromPark, static_cast<int>(_guestRosterParent),
                _guestRosterRideParent
            };
        }
        void openStaffPatrolOptions()
        {
            page(Page::staffPatrolOptions);
        }
        void openGuest(const Handheld::GuestManagement::Target& person)
        {
            _guestRideInspection = false;
            _guestTabs = {};
            _guestFromPark = true;
            bool restored = false;
            if (const auto context = Handheld::PeopleNavigation::guest; context && context->target == person)
            {
                _guestTabs = context->tabs;
                _guestRosterFocus = context->rosterFocus;
                _guestFilter = context->filter;
                _guestFromPark = context->fromPark;
                _guestRosterParent = static_cast<Page>(context->rosterParent);
                _guestRosterRideParent = context->rideParent;
                restored = true;
            }
            Handheld::PeopleNavigation::guest.reset();
            _guestTarget = person;
            _dashboard.section = Handheld::Dashboard::Section::guests;
            _dashboard.focus = Handheld::Dashboard::Focus::content;
            page(Page::guestInspect);
            // A guest picked at the cursor is already on screen, so Locate is a no-op; start on Follow instead.
            if (!restored)
            {
                const auto visible = rows();
                const auto follow = std::find(visible.begin(), visible.end(), "Follow guest");
                if (follow != visible.end())
                {
                    _focus = {};
                    _focus.move(static_cast<int>(follow - visible.begin()), visible.size());
                }
            }
        }
        void openPeepOptions()
        {
            page(Page::peepOptions);
        }
        void openStaff(std::optional<Handheld::StaffManagement::Target> target)
        {
            _staffTabs = {};
            _staffFromPark = target.has_value();
            if (const auto context = Handheld::PeopleNavigation::staff; target && context && context->target == *target)
            {
                _staffTabs = context->tabs;
                _staffRosterFocus = context->rosterFocus;
                _staffFilter = context->filter;
                _staffFromPark = context->fromPark;
            }
            Handheld::PeopleNavigation::staff.reset();
            _staffTarget = target;
            if (target)
                if (const auto* staff = Handheld::StaffManagement::Resolve(*target))
                    _ordersBaseline = _ordersDraft = staff->staffOrders;
            if (!target)
                _staffRosterFocus = 0;
            _dashboard.section = Handheld::Dashboard::Section::staff;
            _dashboard.focus = Handheld::Dashboard::Focus::content;
            page(target ? Page::staffInspect : Page::staff);
        }
        Handheld::ManagementPresentation staffModel() const
        {
            return staffPresentation();
        }
        bool staffIsOpen() const
        {
            return staffManagement();
        }
        void staffPause()
        {
            if (staffManagement())
                page(Page::park);
        }
        void staffHired(const GameActions::Result& result)
        {
            if (!_staffBusy || !staffManagement())
                return;
            _staffBusy = false;
            if (result.error != GameActions::Status::ok)
                _status = Handheld::Finances::Feedback(result);
            else
            {
                _staffTabs = {};
                _staffFromPark = false;
                _staffTarget = Handheld::StaffManagement::Identify(
                    result.getData<GameActions::StaffHireNewActionResult>().StaffEntityId);
                if (_hireManual && _staffTarget)
                {
                    const auto employee = *_staffTarget;
                    saveStaffReturn();
                    close();
                    if (!Handheld::BeginPeepPlacement({ {}, employee }))
                        HandheldStaffOpen(employee);
                    return;
                }
                page(_staffTarget ? Page::staffInspect : Page::staff);
            }
            invalidate();
        }
        Handheld::ManagementPresentation managementPresentation() const
        {
            if (_leaveAction)
                return leavePresentation();
            if (_page == Page::finances)
                return financeWorkspace();
            if (overviewManagement())
                return overviewPresentation();
            if (researchManagement())
                return researchPresentation();
            if (marketingManagement())
                return marketingPresentationFor(_page, _focus.index());
            if (financeManagement())
                return Handheld::Finances::Presentation(
                    _page == Page::financeEditor ? Handheld::Finances::View::editor : Handheld::Finances::View::confirmation,
                    _focus.index(), _financeDraft, _status);
            if (parkInformation())
                return parkInformationPresentation();
            return {};
        }
        bool managementContext()
        {
            if (_leaveAction)
                return true;
            if (_page != Page::research || _researchTabs.tab != 1 || !_researchDraft)
                return false;
            if (Handheld::ResearchManagement::FundingAvailable())
            {
                Handheld::ResearchManagement::AdjustFunding(*_researchDraft, _researchDraft->funding == 3 ? -3 : 1);
                _status.clear();
            }
            else
                _status = "Funding unavailable";
            invalidate();
            return true;
        }
        bool financeIsOpen() const
        {
            return financeManagement();
        }
        void financeMove(int x, int y)
        {
            if (!financeManagement())
                return;
            if (y != 0)
                move(y);
            if (_page == Page::financeEditor && _financeDraft && x != 0)
            {
                Handheld::Finances::Adjust(*_financeDraft, x);
                _status.clear();
                invalidate();
            }
        }
        void financePause()
        {
            if (_civicBusy)
                return;
            if (financeManagement())
                page(Page::park);
        }
        bool keyboardPage() const
        {
            return _page == Page::saveName || _page == Page::sceneryName || _page == Page::scenerySearch
                || _page == Page::ridesName || _page == Page::designName || _page == Page::staffName || _page == Page::guestName
                || _page == Page::guestFilterName;
        }
        // Console keyboards put the common keys on face buttons. Each shortcut presses the real key
        // (focus it, confirm, restore focus), so every screen's own handling still runs.
        bool textEntryKey(Handheld::Action action)
        {
            if (!keyboardPage())
                return false;
            using T = Handheld::TextEntry;
            const size_t key = action == Handheld::Action::contextSecondary ? T::kDelete
                : action == Handheld::Action::contextPrimary                ? T::kSpace
                : action == Handheld::Action::previousOption                ? T::kCase
                : action == Handheld::Action::openMenu                      ? T::kDone
                                                                            : SIZE_MAX;
            if (key == SIZE_MAX)
                return false;
            const auto count = rows().size();
            if (key >= count)
                return false;
            const auto saved = _focus.index();
            _focus.move(static_cast<int>(key) - static_cast<int>(saved), count);
            confirm();
            if (keyboardPage() && key != T::kDone)
                _focus.move(static_cast<int>(saved) - static_cast<int>(_focus.index()), rows().size());
            invalidate();
            return true;
        }
        bool shopIsOpen() const
        {
            return shopManagement();
        }
        void shopMove(int x, int y)
        {
            if (!shopManagement())
                return;
            if (_page == Page::shopAppearanceEditor && _shopAppearanceDraft
                && Handheld::RideManagement::IsColourSetting(_shopAppearanceDraft->setting))
            {
                const auto next = Handheld::MoveColourGrid(_focus.index(), rows().size(), x, y);
                _focus.move(static_cast<int>(next) - static_cast<int>(_focus.index()), rows().size());
                invalidate();
                return;
            }
            if (_page == Page::shop)
            {
                controlsThenGridMove(x, y, 1 + shopPriceCount(), [this](size_t row, int dx) {
                    if (row == 0)
                        setShopOpen(dx > 0);
                    else
                        adjustShopPrice(row - 1, dx);
                });
                return;
            }
            if (y != 0)
                move(y);
        }
        void setShopOpen(bool open)
        {
            const auto* ride = _shop ? Handheld::ShopManagement::Find(*_shop) : nullptr;
            if (!ride || (ride->status == RideStatus::open) == open)
                return;
            const auto result = Handheld::ShopManagement::SetStatus(*_shop, open);
            _status = result.error == GameActions::Status::ok ? "" : result.getErrorMessage();
            invalidate();
        }
        void adjustShopPrice(size_t index, int direction)
        {
            if (!_shop)
                return;
            if (getGameState().park.flags.has(ParkFlag::noMoney))
            {
                _status = "This park does not charge";
                invalidate();
                return;
            }
            const auto prices = Handheld::ShopManagement::Prices(*_shop);
            if (index >= prices.size())
                return;
            const auto next = std::clamp<money64>(prices[index].value + direction, kRideMinPrice, kRideMaxPrice);
            if (next == prices[index].value)
                return;
            const auto result = Handheld::ShopManagement::SetPrice(*_shop, prices[index].primary, next);
            _status = result.error == GameActions::Status::ok ? "" : result.getErrorMessage();
            invalidate();
        }
        void shopPause()
        {
            pauseManagement();
        }
        bool trackSelectionMove(int x, int y)
        {
            if (_page == Page::trackContext && !_trackTechnical)
            {
                _focus.move(x ? x : y, rows().size());
                invalidate();
                return true;
            }
            if (_page != Page::trackBlocks)
                return false;
            if (x && _focus.index() < _trackBlocks.size())
            {
                auto next = x < 0 ? Handheld::TrackEditing::Previous(_trackBlocks[_focus.index()])
                                  : Handheld::TrackEditing::Next(_trackBlocks[_focus.index()]);
                if (next)
                {
                    auto it = std::find(_trackBlocks.begin(), _trackBlocks.end(), *next);
                    if (it != _trackBlocks.end())
                        _focus.move(
                            static_cast<int>(it - _trackBlocks.begin()) - static_cast<int>(_focus.index()), rows().size());
                }
                else
                    _status = x < 0 ? "Start of this section" : "End of this section";
            }
            if (y)
            {
                _focus.move(y, rows().size());
                _status.clear();
            }
            invalidate();
            return true;
        }
        void drawAccessSelection(RenderTarget& rt)
        {
            const auto* point = _page == Page::accessManage
                ? (_focus.index() < _accessPoints.size() ? &_accessPoints[_focus.index()] : nullptr)
                : (_accessPoint ? &*_accessPoint : nullptr);
            auto* main = WindowGetMain();
            if (point && Handheld::RideAccess::Current(*point) && main && main->viewport)
            {
                auto& view = *main->viewport;
                main->savedViewPos = Translate3DTo2DWithZ(
                                         view.rotation, { point->source.x + 16, point->source.y + 16, point->source.z })
                    - ScreenCoordsXY{ view.zoom.ApplyTo(ContextGetWidth() * 3 / 4 - view.pos.x),
                                      view.zoom.ApplyTo(ContextGetHeight() / 2 - view.pos.y) };
                view.viewPos = main->savedViewPos;
                main->flags.unset(WindowFlag::scrollingToLocation);
                const auto oldFlags = gMapSelectFlags;
                const auto oldType = gMapSelectType;
                const auto oldA = gMapSelectPositionA, oldB = gMapSelectPositionB;
                gMapSelectFlags = { MapSelectFlag::enable };
                gMapSelectType = MapSelectType::full;
                gMapSelectPositionA = gMapSelectPositionB = CoordsXY(point->source);
                ViewportRender(rt, main->viewport);
                gMapSelectFlags = oldFlags;
                gMapSelectType = oldType;
                gMapSelectPositionA = oldA;
                gMapSelectPositionB = oldB;
            }
            auto model = ridesPresentationFor(_page, _focus.index());
            model.breadcrumb = "Ride / Entrances & exits";
            model.rowHeight = 34;
            if (_page == Page::accessManage)
            {
                model.title = point ? Handheld::RideAccess::PointLabel(*point) : "Entrances & exits";
                model.bodyLines = { "Selected point is shown in the park." };
            }
            Handheld::DrawManagementContent(rt, { 8, 36 }, { 248, 312 }, model);
            Handheld::DrawFooter(
                rt, { ContextGetWidth(), ContextGetHeight() }, { { "D-pad", "Choose" }, { "A", "Select" }, { "B", "Back" } });
        }
        void drawTrackSelection(RenderTarget& rt)
        {
            const auto* block = _page == Page::trackBlocks
                ? (_focus.index() < _trackBlocks.size() ? &_trackBlocks[_focus.index()] : nullptr)
                : (_trackBlock ? &*_trackBlock : nullptr);
            if (block && Handheld::TrackEditing::Valid(*block))
            {
                auto* main = WindowGetMain();
                if (main && main->viewport)
                {
                    auto& view = *main->viewport;
                    main->savedViewPos = Translate3DTo2DWithZ(
                                             view.rotation, { block->origin.x + 16, block->origin.y + 16, block->origin.z })
                        - ScreenCoordsXY{ view.zoom.ApplyTo(ContextGetWidth() * 3 / 4 - view.pos.x),
                                          view.zoom.ApplyTo(ContextGetHeight() / 2 - view.pos.y) };
                    view.viewPos = main->savedViewPos;
                    main->flags.unset(WindowFlag::scrollingToLocation);
                    // Highlight only for this synchronous native viewport draw. Never retain map mutations.
                    std::vector<std::pair<TrackElement*, bool>> highlights;
                    const auto& data = TrackMetadata::GetTrackElementDescriptor(block->type).sequenceData;
                    for (uint8_t i = 0; i < data.numSequences; ++i)
                    {
                        const auto& c = data.sequences[i].clearance;
                        auto xy = CoordsXY(block->origin) + CoordsXY{ c.x, c.y }.rotate(block->origin.direction);
                        for (auto* e : TileElementsView<TrackElement>(xy))
                            if (!e->isGhost() && e->getRideIndex() == block->ride.id && e->getTrackType() == block->type
                                && e->getDirection() == block->origin.direction && e->getSequenceIndex() == i
                                && e->getBaseZ() == block->origin.z + c.z)
                            {
                                highlights.emplace_back(e, e->isHighlighted());
                                e->setHighlight(true);
                            }
                    }
                    ViewportRender(rt, main->viewport);
                    for (auto [element, original] : highlights)
                        element->setHighlight(original);
                }
            }
            using namespace Handheld;
            if (_page == Page::trackContext && !_trackTechnical && block && TrackEditing::Valid(*block))
            {
                DrawTrackConnections(rt, *block, _focus.index(), _status);
                return;
            }
            if (_page == Page::trackBlocks)
            {
                Concept::Box(rt, { 8, 36 }, { 216, 242 });
                Concept::Heading(rt, { 18, 48 }, "Select track");
                DrawWrappedLabel(rt, { 18, 80 }, 196, block ? CustomTracks::PieceName(block->type) : "Back to ride", 3);
                if (block)
                {
                    Drawing::RenderTarget preview;
                    SelectedLiftAndInverted trackFlags{};
                    if (block->chain)
                        trackFlags.set(LiftHillAndInverted::liftHill);
                    if (block->inverted)
                        trackFlags.set(LiftHillAndInverted::inverted);
                    if (ClipRenderTarget(preview, rt, { 20, 122 }, 192, 92))
                        DrawNativeTrackThumbnail(preview, block->ride.id, block->type, block->origin.direction, trackFlags);
                    DrawModernText(
                        rt, { 18, 220 },
                        "{MUTED}Piece " + std::to_string(_focus.index() + 1) + " / " + std::to_string(_trackBlocks.size()));
                }
                DrawModernEllipsised(rt, { 18, 248 }, 196, _status.empty() ? "{WHITE}A Edit selected piece" : _status);
                DrawFooter(
                    rt, { ContextGetWidth(), ContextGetHeight() },
                    { { "L1/R1", "Connected" }, { "D-pad", "Browse" }, { "A", "Edit" }, { "B", "Ride" } });
            }
            else
            {
                const bool connections = _page == Page::trackContext;
                auto model = ridesPresentationFor(connections ? Page::trackContext : Page::trackBlock, _focus.index());
                model.title = connections ? "Connections" : block ? CustomTracks::PieceName(block->type) : "Track unavailable";
                model.breadcrumb = "Ride / Track piece";
                model.stats.clear();
                model.rowHeight = 24;
                for (auto& row : model.rows)
                {
                    if (row == "Locate block")
                        row = "Locate piece";
                    else if (row == "Remove block...")
                        row = "Remove piece...";
                    else if (row == "Back to blocks")
                        row = "Choose another piece";
                    else if (row == "Replace block...")
                        row = "Replace piece...";
                    else if (row == "Inspect connections / station...")
                        row = "Connections / station details";
                }
                DrawManagementContent(rt, { 8, 36 }, { 240, 312 }, model);
                if (connections && model.selected < model.rows.size() - 2)
                {
                    DrawSurface(rt, { 264, 272 }, { 240, 76 });
                    DrawWrappedLabel(rt, { 276, 288 }, 216, model.rows[model.selected], 3);
                }
                std::vector<ControlHint> hints{ { "D-pad", connections ? "Read" : "Choose" } };
                if (!connections || model.selected + 2 >= model.rows.size())
                    hints.push_back({ "A", "Select" });
                hints.push_back({ "B", connections ? "Piece" : "Pieces" });
                DrawFooter(rt, { ContextGetWidth(), ContextGetHeight() }, hints);
            }
        }
        void move(int32_t direction)
        {
            if (dashboardRailFocused())
            {
                _dashboard.move(direction);
                _status.clear();
                invalidate();
                return;
            }
            const auto visible = rows();
            const auto count = visible.size();
            if (count)
            {
                auto next = std::clamp<int64_t>(
                    static_cast<int64_t>(_focus.index()) + direction, 0, static_cast<int64_t>(count) - 1);
                // Hidden "Back to ..." rows (B does that) are stepped over, unless nothing else is left.
                if (managementShell() && direction != 0)
                {
                    const int step = direction > 0 ? 1 : -1;
                    auto probe = next;
                    while (probe >= 0 && probe < static_cast<int64_t>(count) && Handheld::IsBackRow(visible[probe]))
                        probe += step;
                    if (probe >= 0 && probe < static_cast<int64_t>(count))
                        next = probe;
                    else if (Handheld::IsBackRow(visible[next]))
                        next = _focus.index();
                }
                _focus.move(static_cast<int32_t>(next) - static_cast<int32_t>(_focus.index()), count);
            }
            if ((_page == Page::parkFiles || _page == Page::saveFiles || _page == Page::designFiles))
            {
                _fileFocus = _focus.index();
                _fileDetail = 0;
            }
            _status.clear();
            invalidate();
        }
        void back()
        {
            _menuFeel.MarkBack();
            _feelBack = true;
            if (_leaveAction)
            {
                cancelLeave();
                return;
            }
            if (guardLeave([this] { back(); }))
                return;
            if (_civicBusy)
                return;
            if (_guestRideInspection && (_page == Page::ridesInspect || _page == Page::ridesRoster))
            {
                _guestRideInspection = false;
                _dashboard.section = Handheld::Dashboard::Section::guests;
                _dashboard.focus = Handheld::Dashboard::Focus::content;
                page(Page::guestRideHistory);
                return;
            }
            if (_managementNewsReturn
                && (_page == Page::finances || _page == Page::marketing || _page == Page::researchItem
                    || _page == Page::researchCatalog || _page == Page::guestRoster || _page == Page::overview
                    || _page == Page::parkAwards))
            {
                _managementNewsReturn = false;
                _dashboard.section = Handheld::Dashboard::Section::overview;
                _dashboard.focus = Handheld::Dashboard::Focus::content;
                page(Page::parkNewsDetail);
                return;
            }
            if (_mapInspection
                && (_page == Page::guestInspect || _page == Page::staffInspect || _page == Page::ridesInspect
                    || _page == Page::guestRoster || _page == Page::staff || _page == Page::ridesRoster || _page == Page::shop))
            {
                _mapInspection = false;
                _dashboard.section = Handheld::Dashboard::Section::overview;
                _dashboard.focus = Handheld::Dashboard::Focus::content;
                page(_mapInspectionReturn);
                return;
            }
            switch (_page)
            {
                case Page::primer:
                    finishPrimer();
                    break;
                case Page::controls:
                    page(Page::park);
                    _focus.move(7, rows().size());
                    break;
                case Page::settings:
                    page(Page::park);
                    _focus.move(14, rows().size());
                    break;
                case Page::researchCatalog:
                    page(Page::research);
                    break;
                case Page::researchItem:
                    page(Page::researchCatalog);
                    _focus.move(static_cast<int>(_researchCatalogFocus), rows().size());
                    break;
                case Page::financeHistory:
                case Page::financeLedger:
                    page(Page::finances);
                    break;
                case Page::mapSubject:
                    page(Page::mapSubjects);
                    _focus.move(static_cast<int>(_mapSubjectsFocus), rows().size());
                    break;
                case Page::mapSubjects:
                    page(Page::parkMap);
                    break;
                case Page::parkMap:
                    if (_guestMapReturn)
                    {
                        _guestMapReturn = false;
                        _dashboard.section = Handheld::Dashboard::Section::guests;
                        page(Page::guestRoster);
                        _focus.move(static_cast<int>(_guestRosterFocus), rows().size());
                    }
                    else
                    {
                        _dashboard.section = Handheld::Dashboard::Section::overview;
                        page(Page::overview);
                    }
                    break;
                case Page::parkNewsDetail:
                    if (_newsFromPark)
                    {
                        close();
                        break;
                    }
                    page(Page::parkNews);
                    _focus.move(static_cast<int>(_parkNewsFocus), rows().size());
                    break;
                case Page::parkNews:
                    if (_newsFromPark)
                    {
                        close();
                        break;
                    }
                    if (_newsFromDashboard)
                    {
                        _dashboard = _newsDashboard;
                        page(Page::dashboard);
                        break;
                    }
                    [[fallthrough]];
                case Page::parkAwards:
                case Page::parkHistory:
                    _dashboard.section = Handheld::Dashboard::Section::overview;
                    page(Page::overview);
                    break;
                case Page::sceneryCategory:
                    page(Page::build);
                    break;
                case Page::sceneryPicker:
                    if (_sceneryPickerDirect)
                        close();
                    else
                        page(Page::sceneryOptions);
                    break;
                case Page::rightsConfirm:
                    _rightsDraft.reset();
                    close();
                    break;
                case Page::staffPatrolClear:
                    page(Page::staffPatrolOptions);
                    break;
                case Page::peepCancel:
                    page(Page::peepOptions);
                    break;
                case Page::peepOptions:
                    close();
                    return;
                case Page::staffPatrolOptions:
                    close();
                    break;
                case Page::rightsOptions:
                    close();
                    break;
                case Page::areaOptions:
                    close();
                    break;
                case Page::scenerySearch:
                    page(_scenerySearchParent);
                    if (_scenerySearchParent == Page::sceneryPicker)
                        refreshPicker();
                    else
                        _focus.move(static_cast<int>(_scenerySearchRow), rows().size());
                    break;
                case Page::sceneryName:
                    leaveSign();
                    break;
                case Page::sceneryConfirm:
                    page(Page::sceneryName);
                    break;
                case Page::terrainPaintConfirm:
                    _paintDraft.reset();
                    close();
                    break;
                case Page::sceneryClearConfirm:
                    _clearDraft.reset();
                    close();
                    break;
                case Page::sceneryClearOptions:
                    close();
                    break;
                case Page::sceneryGroups:
                case Page::sceneryStyle:
                    page(Page::sceneryOptions);
                    break;
                case Page::sceneryStyleEditor:
                    page(Page::sceneryStyle);
                    break;
                case Page::sceneryStyleConfirm:
                    page(Page::sceneryStyleEditor);
                    break;
                case Page::sceneryOptions:
                    close();
                    break;
                case Page::dashboard:
                    close();
                    break;
                case Page::designOptions:
                    close();
                    break;
                case Page::designConfirm:
                    Handheld::Designs::CancelReview();
                    close();
                    break;
                case Page::customChoice:
                    openRidePicker(true);
                    break;
                case Page::trackPiecePicker:
                    close();
                    break;
                case Page::designPicker:
                    if (_designPickerFromBuild)
                        page(Page::build);
                    else
                        close();
                    break;
                case Page::ridePicker:
                    if (_ridePickerFromBuild)
                        page(Page::build);
                    else
                        close();
                    break;
                case Page::shopPicker:
                    if (_shopPickerFromBuild)
                        page(Page::build);
                    else
                        close();
                    break;
                case Page::shopAppearanceEditor:
                    page(Page::shopAppearance);
                    _focus.move(static_cast<int>(_shopAppearanceChannel), rows().size());
                    break;
                case Page::shopAppearanceReview:
                {
                    page(Page::shopAppearanceEditor);
                    if (_shopAppearanceDraft)
                    {
                        const auto& d = *_shopAppearanceDraft;
                        const auto it = std::find(d.setting.values.begin(), d.setting.values.end(), d.value);
                        if (it != d.setting.values.end())
                            _focus.move(static_cast<int>(it - d.setting.values.begin()), rows().size());
                    }
                    break;
                }
                case Page::shopAppearance:
                case Page::shopDemolish:
                    page(Page::shop);
                    break;
                case Page::terrainMaterialPicker:
                    if (_terrainMaterialFromSettings)
                        page(Page::areaOptions);
                    else
                        close();
                    break;
                case Page::pathSurfacePicker:
                    if (_pathSurfaceDirect)
                        close();
                    else
                        page(Page::pathOptions);
                    break;
                case Page::pathUndoConfirm:
                    _pathUndo.reset();
                    page(Page::pathOptions);
                    break;
                case Page::shop:
                case Page::ride:
                case Page::pathOptions:
                case Page::treePicker:
                    close();
                    break;
                case Page::ridesRoster:
                    page(Page::dashboard);
                    break;
                case Page::trackBlocks:
                    page(Page::ridesInspect);
                    break;
                case Page::trackBlock:
                    if (_ridesTarget)
                        _trackBlocks = Handheld::TrackEditing::Blocks(*_ridesTarget);
                    page(Page::trackBlocks);
                    break;
                case Page::trackContext:
                case Page::trackRemoveConfirm:
                case Page::trackReplaceConfirm:
                case Page::trackSpeedConfirm:
                    page(Page::trackBlock);
                    break;
                case Page::ridesInspect:
                    if (_rideFromPark)
                        close();
                    else
                        page(Page::ridesRoster);
                    break;
                case Page::designExport:
                    Handheld::DesignAuthoring::CancelReview();
                    page(Page::designName);
                    break;
                case Page::designName:
                    page(Page::designScenery);
                    break;
                case Page::designScenery:
                    page(Page::ridesInspect);
                    break;
                case Page::ridesName:
                    page(_ridesTarget && _ridesTarget->shop ? Page::shop : Page::ridesInspect);
                    break;
                case Page::accessManage:
                    page(Page::ridesInspect);
                    break;
                case Page::accessPoint:
                    openAccessManagement();
                    break;
                case Page::accessReview:
                    if (!_ridesBusy)
                        cancelAccessReview();
                    break;
                case Page::ridesAccess:
                case Page::ridesVehicles:
                case Page::ridesAppearance:
                case Page::ridesMusic:
                case Page::ridesDispatch:
                case Page::ridesOperations:
                case Page::ridesMaintenance:
                case Page::ridesPrice:
                    page(ridesDraftParent());
                    break;
                case Page::ridesStations:
                    page(Page::ridesCustomers);
                    break;
                case Page::ridesGraphs:
                case Page::ridesCustomers:
                case Page::ridesEarnings:
                    page(_ridesTabChild ? Page::ridesInspect : Page::ridesStats);
                    break;
                case Page::ridesStats:
                    page(
                        _ridesTarget && Handheld::RideManagement::Resolve(*_ridesTarget) ? Page::ridesInspect
                                                                                         : Page::ridesRoster);
                    break;
                case Page::ridesConfirm:
                    if (_ridesDraft
                        && (_ridesDraft->renaming || _ridesDraft->refurbishment || !_ridesDraft->randomColours.empty()))
                        page(ridesDraftParent());
                    else if (_ridesDraft && !_ridesDraft->status && !_ridesDraft->demolition)
                        page(Page::ridesPrice);
                    else
                        page(
                            _ridesTarget && Handheld::RideManagement::Resolve(*_ridesTarget) ? Page::ridesInspect
                                                                                             : Page::ridesRoster);
                    break;
                case Page::guestGroups:
                    page(Page::guests);
                    break;
                case Page::guestRoster:
                    if (_guestRosterParent == Page::ridesCustomers)
                    {
                        _dashboard.section = Handheld::Dashboard::Section::rides;
                        _ridesTarget = _guestRosterRideParent;
                    }
                    page(_guestRosterParent);
                    break;
                case Page::guestInspect:
                    if (_guestFromPark)
                        close();
                    else
                        page(Page::guestRoster);
                    break;
                case Page::guestText:
                    page(
                        _guestTarget && Handheld::GuestManagement::Resolve(*_guestTarget) ? _guestTextParent
                                                                                          : Page::guestRoster);
                    break;
                case Page::guestFilterName:
                    page(Page::guestRoster);
                    break;
                case Page::guestNameConfirm:
                    page(Page::guestName);
                    break;
                case Page::guestName:
                case Page::guestTracking:
                case Page::guestRideHistory:
                case Page::guestPreferences:
                case Page::guestNeeds:
                case Page::guestThoughts:
                case Page::guestSpending:
                case Page::guestInventory:
                    page(
                        _guestTarget && Handheld::GuestManagement::Resolve(*_guestTarget) ? Page::guestInspect
                                                                                          : Page::guestRoster);
                    break;
                case Page::staffRoles:
                case Page::staffHire:
                    page(Page::staff);
                    break;
                case Page::staffInspect:
                    if (_staffFromPark)
                        close();
                    else
                        page(Page::staff);
                    break;
                case Page::staffNameConfirm:
                    page(Page::staffName);
                    break;
                case Page::staffSummary:
                    page(Page::staff);
                    break;
                case Page::staffUniform:
                case Page::staffName:
                case Page::staffStats:
                case Page::staffOrders:
                case Page::staffCostumes:
                case Page::staffDismiss:
                    page(Page::staffInspect);
                    break;
                case Page::staff:
                    page(Page::dashboard);
                    break;
                case Page::marketing:
                    page(Page::dashboard);
                    break;
                case Page::marketingActive:
                case Page::marketingTypes:
                    page(Page::marketing);
                    break;
                case Page::marketingEditor:
                    _marketingTabs.tab = 1;
                    page(Page::marketing);
                    break;
                case Page::marketingTargets:
                case Page::marketingConfirm:
                    page(Page::marketingEditor);
                    break;
                case Page::researchConfirm:
                    page(Page::research);
                    break;
                case Page::researchEditor:
                    page(Page::research);
                    break;
                case Page::financeConfirm:
                    page(Page::financeEditor);
                    break;
                case Page::financeEditor:
                    page(_financeReturnPage);
                    break;
                case Page::overviewConfirm:
                    page(Page::overview);
                    break;
                case Page::overview:
                    if (_objectivesFromPark)
                    {
                        _objectivesFromPark = false;
                        // A new scenario: the first time ever, the controls intro follows the goal.
                        Handheld::MenuThemes::settings.Load(
                            GetContext()->GetPlatformEnvironment().GetDirectoryPath(DirBase::user));
                        if (!Handheld::preferences.primerSeen)
                            openPrimer(false);
                        else
                            close();
                        break;
                    }
                    page(Page::dashboard);
                    break;
                case Page::finances:
                case Page::guests:
                case Page::research:
                    page(Page::dashboard);
                    break;
                case Page::paths:
                    page(Page::build);
                    break;
                case Page::overwrite:
                    page(Page::save);
                    break;
                case Page::parkInspect:
                    page(Page::parkFiles);
                    break;
                case Page::designVehicles:
                    page(_designVehicleFromFile ? Page::designInspect : Page::designOptions);
                    break;
                case Page::designWarnings:
                    page(_designDetailsFromFile ? Page::designInspect : Page::designOptions);
                    break;
                case Page::designInspect:
                    _designMetadata.reset();
                    _designFilePreview.reset();
                    page(Page::designFiles);
                    break;
                case Page::designFiles:
                    page(Page::designPicker);
                    refreshPicker();
                    break;
                case Page::saveFiles:
                    _saveDraft.reset();
                    page(Page::save);
                    break;
                case Page::saveName:
                    _saveDraft.reset();
                    page(Page::saveFiles);
                    break;
                case Page::saveReview:
                    _saveDraft.reset();
                    page(Page::saveName);
                    break;
                case Page::parkFiles:
                    page(Page::load);
                    break;
                case Page::discard:
                    page(_fileLoad ? Page::parkFiles : Page::load);
                    break;
                case Page::exit:
                    page(Page::quitChoice);
                    _focus = {};
                    _focus.move(1, 3);
                    break;
                case Page::quitChoice:
                    cancelQuit();
                    break;
                case Page::save:
                    if (_quitSaving)
                    {
                        page(Page::quitChoice);
                        _quitSaving = false;
                    }
                    else
                        page(Page::park);
                    break;
                case Page::build:
                    close();
                    break;
                case Page::park:
                    close(); // Back restores the pause state that existed before this menu.
                    break;
                default:
                    if (gLegacyScene == LegacyScene::playing)
                        page(Page::park);
                    else
                        close();
                    break;
            }
        }
        void applyShopAppearanceDraft()
        {
            if (_shopAppearanceDraft)
            {
                const auto q = Handheld::ShopAppearance::Action(*_shopAppearanceDraft, false);
                if (q.error != GameActions::Status::ok)
                {
                    _status = q.getErrorMessage();
                    invalidate();
                    return;
                }
                _shopAppearanceBusy = true;
                const auto request = ++_shopAppearanceRequest;
                const auto result = Handheld::ShopAppearance::Action(
                    *_shopAppearanceDraft, true,
                    [this, request,
                     lifetime = std::weak_ptr<int>(_shopAppearanceLifetime)](const auto*, const GameActions::Result* outcome) {
                        if (lifetime.expired() || flags.has(WindowFlag::dead) || request != _shopAppearanceRequest
                            || (_page != Page::shopAppearanceEditor && _page != Page::shopAppearanceReview))
                            return;
                        _shopAppearanceBusy = false;
                        if (outcome->error == GameActions::Status::ok)
                        {
                            page(Page::shopAppearance);
                            _focus.move(static_cast<int>(_shopAppearanceChannel), rows().size());
                            _status = "Appearance updated";
                        }
                        else
                            _status = outcome->getErrorMessage();
                        invalidate();
                    });
                if (result.error != GameActions::Status::ok)
                {
                    _shopAppearanceBusy = false;
                    _status = result.getErrorMessage();
                }
                invalidate();
            }
        }
        void applyRideDraft()
        {
            if (_ridesDraft)
            {
                const auto query = Handheld::RideManagement::Action(*_ridesDraft, false);
                if (query.error != GameActions::Status::ok)
                {
                    _status = Handheld::Finances::Feedback(query);
                    invalidate();
                    return;
                }
                const auto expectedPage = _page;
                const auto request = ++_ridesRequest;
                _ridesBusy = true;
                const auto result = Handheld::RideManagement::Action(
                    *_ridesDraft, true,
                    [this, request, expectedPage, lifetime = std::weak_ptr<int>(_ridesLifetime)](
                        const GameActions::GameAction*, const GameActions::Result* outcome) {
                        if (lifetime.expired() || request != _ridesRequest || _page != expectedPage)
                            return;
                        _ridesBusy = false;
                        if (outcome->error == GameActions::Status::ok)
                        {
                            const bool demolished = _ridesDraft && _ridesDraft->demolition;
                            // A native vehicle-object change intentionally changes the identity's object field.
                            // Refresh only after this owned action succeeds; external changes remain stale.
                            if (_ridesDraft && _ridesDraft->setting
                                && _ridesDraft->setting->vehicle == GameActions::RideSetVehicleType::rideEntry)
                                _ridesTarget = Handheld::RideManagement::Identify(_ridesDraft->target.id);
                            if (_ridesDraft && _ridesDraft->refurbishment)
                                _ridesTarget = Handheld::RideManagement::Identify(_ridesDraft->target.id);
                            const auto destination = _ridesDraft && _ridesDraft->renaming
                                ? (_ridesTarget && _ridesTarget->shop ? Page::shop : Page::ridesInspect)
                                : ridesDraftParent();
                            page(demolished ? Page::ridesRoster : destination);
                            _status = demolished ? "Ride demolished" : "Ride updated";
                        }
                        else
                            _status = Handheld::Finances::Feedback(*outcome);
                        invalidate();
                    });
                if (result.error != GameActions::Status::ok)
                {
                    _ridesBusy = false;
                    _status = Handheld::Finances::Feedback(result);
                }
                invalidate();
            }
        }
        void applyFinanceDraft()
        {
            if (_financeDraft)
            {
                const auto expectedPage = _page;
                _civicBusy = true;
                const auto result = Handheld::Finances::Action(
                    *_financeDraft, true,
                    [this, expectedPage,
                     lifetime = std::weak_ptr<int>(_civicLifetime)](const auto*, const GameActions::Result* outcome) {
                        if (lifetime.expired())
                            return;
                        _civicBusy = false;
                        if (_page != expectedPage)
                            return;
                        if (outcome->error == GameActions::Status::ok)
                        {
                            page(_financeReturnPage);
                            _status = "Financial change applied";
                        }
                        else
                            _status = Handheld::Finances::Feedback(*outcome);
                        invalidate();
                    });
                if (result.error != GameActions::Status::ok)
                {
                    _civicBusy = false;
                    _status = Handheld::Finances::Feedback(result);
                }
                else if (_civicBusy)
                    _status = "Applying financial change...";
                invalidate();
            }
        }
        void confirm()
        {
            if (_leaveAction)
            {
                confirmLeave();
                return;
            }
            if (_shopAppearanceBusy)
                return;
            if (_civicBusy || (overviewManagement() && _overviewBusy) || (ridesManagement() && _ridesBusy)
                || (marketingManagement() && _marketingBusy))
                return;
            if (ridesManagement() && _page != Page::ridesRoster
                && (!_ridesTarget || !Handheld::RideManagement::Resolve(*_ridesTarget)))
            {
                page(Page::ridesRoster);
                return;
            }
            _menuFeel.Press();
            auto choice = _focus.index();
            if (_page == Page::guestRoster)
                choice = choice < 3                     ? _guestRoster.size() + choice
                    : choice == _guestRoster.size() + 3 ? _guestRoster.size() + 3
                                                        : choice - 3;
            auto actionPage = _page;
            if (_page == Page::staffInspect)
            {
                const auto model = staffPresentation();
                if (choice < model.rowEnabled.size() && !model.rowEnabled[choice])
                {
                    _status = model.rows[choice] == "Costume..."    ? "Only entertainers wear costumes"
                        : model.rows[choice] == "Uniform colour..." ? "Entertainers use costume colours"
                                                                    : "This employee cannot be picked up now";
                    invalidate();
                    return;
                }
                _staffTabs.save(choice);
                if (_staffTabs.tab == 1)
                {
                    if (!_staffTarget || Handheld::StaffManagement::Orders(_staffTarget->type).empty())
                    {
                        _staffTabs.tab = 0;
                        page(Page::staffInspect);
                        return;
                    }
                    actionPage = Page::staffOrders;
                }
                else
                {
                    const auto indices = Handheld::PeopleTabs::StaffActions(_staffTabs.tab);
                    if (choice >= indices.size())
                        return;
                    choice = indices[choice];
                }
            }
            if (_page == Page::guestInspect)
            {
                _guestTabs.save(choice);
                if (_guestTabs.tab == 1)
                {
                    _guestTabs.tab = 0;
                    page(Page::guestInspect);
                    return;
                }
                if (_guestTabs.tab == 2)
                {
                    const auto* guest = _guestTarget ? Handheld::GuestManagement::Resolve(*_guestTarget) : nullptr;
                    const auto thoughts = guest ? Handheld::GuestManagement::Thoughts(*guest) : std::vector<std::string>{};
                    if (choice >= thoughts.size())
                    {
                        _guestTabs.tab = 0;
                        page(Page::guestInspect);
                        return;
                    }
                    _guestText = thoughts[choice];
                    _guestTextParent = Page::guestInspect;
                    _guestTextPage = 0;
                    page(Page::guestText);
                    return;
                }
                const auto indices = Handheld::PeopleTabs::GuestActions(_guestTabs.tab);
                if (choice >= indices.size())
                    return;
                choice = indices[choice];
            }
            switch (actionPage)
            {
                case Page::designOptions:
                    if (choice == 0)
                    {
                        close();
                        break;
                    }
                    if (choice == 5)
                    {
                        openDesignVehicles(false);
                        break;
                    }
                    if (choice == 6)
                    {
                        _designDetailsFromFile = false;
                        _designDetail = 0;
                        page(Page::designWarnings);
                        break;
                    }
                    constructionMove(1, 0);
                    break;
                case Page::designConfirm:
                    if (choice == 0)
                    {
                        Handheld::Designs::CancelReview();
                        close();
                    }
                    else
                    {
                        // Release only the pause owned by this menu before native revalidation.
                        close();
                        const auto result = Handheld::Designs::Confirm();
                        if (result.error != GameActions::Status::ok)
                            GetWindowManager()->ShowError("Cannot place design", result.getErrorMessage());
                    }
                    break;
                case Page::designPicker:
                    if (_picker.selected() && Handheld::BeginSavedDesign(_picker.selected()->identity.key))
                        close();
                    else
                    {
                        refreshPicker();
                        _status = "Design unavailable";
                        invalidate();
                    }
                    break;
                case Page::customChoice:
                    if (choice == 0)
                    {
                        if (Handheld::BeginCustomTrack(_customObject, _customType))
                            close();
                        else
                        {
                            _status = "Ride construction unavailable";
                            invalidate();
                        }
                    }
                    else if (choice == 1)
                        openDesignPicker(true);
                    else
                        openRidePicker(true);
                    break;
                case Page::trackPiecePicker:
                    if (_picker.selected())
                    {
                        const auto key = _picker.selected()->identity.key;
                        if (key == "leave")
                        {
                            const auto target = Handheld::CustomTracks::Target();
                            const auto context = Handheld::RideNavigation::Take();
                            Handheld::EndTerrain();
                            close();
                            if (target)
                                HandheldRideReturn(*target, context);
                        }
                        else if (key == "camera")
                        {
                            Handheld::CustomTracks::ToggleCamera();
                            close();
                        }
                        else if (key == "covered" || key == "inverted")
                        {
                            const auto ok = key == "covered" ? Handheld::CustomTracks::ToggleCovered()
                                                             : Handheld::CustomTracks::ToggleInverted();
                            if (ok)
                                close();
                            else
                            {
                                _status = "Variant cannot connect here";
                                invalidate();
                            }
                        }
                        else if (key.starts_with("seat-"))
                        {
                            const auto currentSeat = Handheld::CustomTracks::PlacementSeat();
                            const auto value = key == "seat-reset" ? 4
                                : key == "seat-minus"              ? std::max<int>(0, currentSeat - 1)
                                                                   : std::min<int>(15, currentSeat + 1);
                            if (Handheld::CustomTracks::SetPlacementSeat(value))
                                close();
                            else
                            {
                                _status = "Seat rotation unavailable";
                                invalidate();
                            }
                        }
                        else if (Handheld::CustomTracks::Select(static_cast<TrackElemType>(std::stoi(key))))
                            close();
                        else
                        {
                            refreshPicker();
                            _status = "Piece unavailable";
                            invalidate();
                        }
                    }
                    break;
                case Page::ridePicker:
                    if (_picker.selected() && _picker.selected()->identity.key.starts_with("maze:"))
                    {
                        if (Handheld::BeginMaze(_picker.selected()->identity.index))
                            close();
                        else
                        {
                            refreshPicker();
                            _status = "Maze unavailable";
                            invalidate();
                        }
                        break;
                    }
                    if (_picker.selected()
                        && (_picker.selected()->identity.key.starts_with("custom:")
                            || _picker.selected()->identity.key.starts_with("custom-type:")))
                    {
                        _customObject = _picker.selected()->identity.index;
                        const auto key = _picker.selected()->identity.key;
                        _customType = key.starts_with("custom-type:")
                            ? static_cast<ride_type_t>(std::stoi(key.substr(12)))
                            : static_cast<ride_type_t>(RIDE_TYPE_WOODEN_ROLLER_COASTER);
                        page(Page::customChoice);
                        break;
                    }
                    if (_picker.selected() && Handheld::BeginFlatRide(_picker.selected()->identity.index))
                        close();
                    else
                    {
                        refreshPicker();
                        _status = "Ride unavailable";
                        invalidate();
                    }
                    break;
                case Page::shopPicker:
                    if (_picker.selected() && Handheld::SetShopSelection(_picker.selected()->identity.index))
                        close();
                    else
                    {
                        refreshPicker();
                        _status = "Shop unavailable";
                        invalidate();
                    }
                    break;
                case Page::terrainMaterialPicker:
                    if (_picker.selected()
                        && Handheld::TerrainMaterials::Select(_terrainMaterialEdge, _picker.selected()->identity))
                    {
                        if (_terrainMaterialFromSettings)
                            page(Page::areaOptions);
                        else
                            close();
                    }
                    else
                    {
                        refreshPicker();
                        _status = "Material changed; select again";
                        invalidate();
                    }
                    break;
                case Page::pathSurfacePicker:
                    if (_picker.selected() && Handheld::PathSurfaces::Select(_picker.selected()->identity))
                        close();
                    else
                    {
                        refreshPicker();
                        _status = "Surface no longer available";
                        invalidate();
                    }
                    break;
                case Page::treePicker:
                    if (_picker.selected() && Handheld::SetTreeSelection(_picker.selected()->identity.index))
                        close();
                    else
                    {
                        refreshPicker();
                        _status = "Tree unavailable";
                        invalidate();
                    }
                    break;
                case Page::primer:
                    if (_focus.index() + 1 < rows().size())
                    {
                        _focus.move(1, rows().size());
                        invalidate();
                    }
                    else
                        finishPrimer();
                    break;
                case Page::controls:
                    page(Page::park);
                    _focus.move(7, rows().size());
                    break;
                case Page::park:
                    switch (choice)
                    {
                        case 0:
                            resumePark();
                            break;
                        case 1:
                            page(Page::build);
                            break;
                        case 2:
                            page(Page::dashboard);
                            break;
                        case 3:
                            _exitAfterSave = false;
                            page(Page::save);
                            break;
                        case 4:
                            _exitAfterSave = true;
                            page(Page::save);
                            break;
                        case 5:
                            page(Page::load);
                            break;
                        case 6:
                            openQuit(false);
                            break;
                        case 7:
                            page(Page::controls);
                            break;
                        case 12:
                            Handheld::MenuThemes::settings.Load(
                                GetContext()->GetPlatformEnvironment().GetDirectoryPath(DirBase::user));
                            _status = Handheld::MenuThemes::settings.Toggle()
                                ? "Theme changed"
                                : "Theme changed for this session; preference could not be saved";
                            invalidate();
                            break;
                        case 13:
                            Handheld::MenuThemes::settings.Load(
                                GetContext()->GetPlatformEnvironment().GetDirectoryPath(DirBase::user));
                            _status = Handheld::MenuThemes::settings.ToggleMotion()
                                ? "Motion changed"
                                : "Motion changed for this session; preference could not be saved";
                            invalidate();
                            break;
                        case 8:
                        case 9:
                        case 10:
                        case 11:
                            chooseSimulation(static_cast<uint8_t>(choice - 8));
                            break;
                        case 14:
                            page(Page::settings);
                            break;
                    }
                    break;
                case Page::settings:
                    toggleSetting(choice);
                    break;
                case Page::sceneryCategory:
                    if (choice == 5)
                    {
                        close();
                        Handheld::BeginSceneryClear();
                    }
                    else if (choice == 6)
                        page(Page::build);
                    else
                    {
                        Handheld::SceneryTools::SelectCategory(static_cast<Handheld::SceneryTools::Category>(choice));
                        page(Page::sceneryPicker);
                        refreshPicker();
                    }
                    break;
                case Page::sceneryPicker:
                    if (_picker.selected()
                        && Handheld::SceneryTools::Select(_picker.selected()->identity.index, _picker.selected()->identity.key))
                    {
                        close();
                        if (!Handheld::IsSceneryToolActive())
                            Handheld::BeginScenery();
                    }
                    else
                    {
                        refreshPicker();
                        _status = "Scenery unavailable";
                    }
                    break;
                case Page::scenerySearch:
                    if (choice == 47)
                        back();
                    else if (choice == 46)
                    {
                        if (Handheld::SceneryTools::SetSearch(_nameEntry.text))
                        {
                            page(Page::sceneryPicker);
                            refreshPicker();
                        }
                        else
                        {
                            _status = "Search is limited to 32 characters";
                            invalidate();
                        }
                    }
                    else
                    {
                        _nameEntry.Press(choice);
                        invalidate();
                    }
                    break;
                case Page::sceneryName:
                    if (choice == 47)
                        leaveSign();
                    else if (choice == 46 && _signTarget)
                    {
                        const auto query = Handheld::SceneryTools::SetText(*_signTarget, _nameEntry.text, true);
                        if (query.error == GameActions::Status::ok)
                        {
                            leaveSign();
                        }
                        else
                        {
                            _status = query.getErrorMessage();
                            invalidate();
                        }
                    }
                    else
                    {
                        _nameEntry.Press(choice);
                        invalidate();
                    }
                    break;
                case Page::sceneryConfirm:
                    if (choice == 1)
                        page(Page::sceneryName);
                    else if (_signTarget)
                    {
                        const auto result = Handheld::SceneryTools::SetText(*_signTarget, _nameEntry.text, true);
                        if (result.error == GameActions::Status::ok)
                            leaveSign();
                        else
                        {
                            _status = result.getErrorMessage();
                            invalidate();
                        }
                    }
                    break;
                case Page::sceneryClearOptions:
                    if (choice == 0)
                        close();
                    else
                    {
                        Handheld::SceneryClear::Configure(choice);
                        invalidate();
                    }
                    break;
                case Page::terrainPaintConfirm:
                    if (choice == 1)
                    {
                        _paintDraft.reset();
                        close();
                    }
                    else if (_paintDraft)
                    {
                        const auto result = Handheld::AreaTools::ApplyPaint(*_paintDraft, true);
                        if (result.error == GameActions::Status::ok)
                        {
                            _paintDraft.reset();
                            close();
                        }
                        else
                        {
                            _status = result.getErrorMessage();
                            invalidate();
                        }
                    }
                    break;
                case Page::sceneryClearConfirm:
                    if (choice == 1)
                    {
                        _clearDraft.reset();
                        close();
                    }
                    else if (_clearDraft)
                    {
                        const auto result = Handheld::SceneryClear::Action(*_clearDraft, true);
                        if (result.error == GameActions::Status::ok)
                        {
                            _clearDraft.reset();
                            close();
                        }
                        else
                        {
                            _status = result.getErrorMessage();
                            invalidate();
                        }
                    }
                    break;
                case Page::sceneryGroups:
                {
                    const auto groups = Handheld::SceneryTools::Groups(Handheld::SceneryTools::Current().category);
                    if (choice < groups.size() && Handheld::SceneryTools::SetGroup(groups[choice].index))
                    {
                        page(Page::sceneryPicker);
                        refreshPicker();
                    }
                    else
                    {
                        _status = "Scenery group changed";
                        invalidate();
                    }
                    break;
                }
                case Page::sceneryStyle:
                {
                    const auto styles = signStyles();
                    if (choice >= styles.size())
                        page(Page::sceneryOptions);
                    else if (_signTarget)
                    {
                        _signStyle = Handheld::SceneryTools::BeginStyle(*_signTarget, styles[choice]);
                        if (_signStyle)
                            page(Page::sceneryStyleEditor);
                        else
                        {
                            _status = "Style unavailable for this sign";
                            invalidate();
                        }
                    }
                    break;
                }
                case Page::sceneryStyleEditor:
                    if (choice == 2)
                        page(Page::sceneryStyle);
                    else if (_signStyle && choice == 0)
                    {
                        Handheld::SceneryTools::AdjustStyle(*_signStyle, 1);
                        invalidate();
                    }
                    else if (_signStyle)
                    {
                        const auto result = Handheld::SceneryTools::ApplyStyle(*_signStyle, false);
                        if (result.error == GameActions::Status::ok)
                        {
                            page(Page::sceneryStyleConfirm);
                            _focus.move(1, 2);
                        }
                        else
                        {
                            _status = result.getErrorMessage();
                            invalidate();
                        }
                    }
                    break;
                case Page::sceneryStyleConfirm:
                    if (choice == 1)
                        page(Page::sceneryStyleEditor);
                    else if (_signStyle)
                    {
                        const auto result = Handheld::SceneryTools::ApplyStyle(*_signStyle, true);
                        if (result.error == GameActions::Status::ok)
                        {
                            _signTarget.reset();
                            _signStyle.reset();
                            page(Page::sceneryOptions);
                        }
                        else
                        {
                            _status = result.getErrorMessage();
                            invalidate();
                        }
                    }
                    break;
                case Page::sceneryOptions:
                    if (!Handheld::ConstructionAdjust::SceneryEnabled(choice))
                    {
                        _status = "Fixed or unavailable for this object";
                        invalidate();
                    }
                    else if (choice == 15)
                        openScenerySearch();
                    else if (choice == 14)
                    {
                        Handheld::SceneryTools::BeginPick();
                        close();
                    }
                    else if (choice == 12)
                        page(Page::sceneryGroups);
                    else if (choice == 13)
                    {
                        _signTarget = Handheld::SceneryEditTarget();
                        if (_signTarget && _signTarget->banner != BannerIndex::GetNull())
                            page(Page::sceneryStyle);
                        else
                        {
                            _status = "Point at a banner or sign";
                            invalidate();
                        }
                    }
                    else if (choice == 11)
                    {
                        _signTarget = Handheld::SceneryEditTarget();
                        if (_signTarget && _signTarget->banner != BannerIndex::GetNull())
                        {
                            _nameEntry = { _signTarget->text };
                            page(Page::sceneryName);
                        }
                        else
                        {
                            _status = "Point at a banner or sign";
                            invalidate();
                        }
                    }
                    else if (choice == 0)
                        close();
                    else if (choice == 1)
                    {
                        page(Page::sceneryPicker);
                        refreshPicker();
                    }
                    else
                    {
                        Handheld::SceneryTools::Configure(choice);
                        invalidate();
                    }
                    break;
                case Page::peepOptions:
                    if (choice == 1)
                        page(Page::peepCancel);
                    else if (choice == 2)
                    {
                        const auto person = Handheld::PeepPlacement::target;
                        close();
                        Handheld::EndTerrain();
                        if (person && person->guest)
                            HandheldGuestOpen(*person->guest);
                        else if (person && person->staff)
                            HandheldStaffOpen(*person->staff);
                        return;
                    }
                    else
                        close();
                    break;
                case Page::peepCancel:
                    if (choice == 1)
                        page(Page::peepOptions);
                    else
                    {
                        close();
                        Handheld::EndTerrain();
                    }
                    break;
                case Page::staffPatrolOptions:
                    if (choice == 0)
                        close();
                    else if (choice == 5)
                    {
                        auto employee = Handheld::StaffPatrol::target;
                        close();
                        Handheld::EndTerrain();
                        if (employee)
                            HandheldStaffOpen(employee);
                    }
                    else if (choice == 4)
                    {
                        _patrolDraft = Handheld::StaffPatrol::ReviewClear();
                        if (_patrolDraft)
                        {
                            page(Page::staffPatrolClear);
                            _focus.move(1, rows().size());
                        }
                        else
                            _status = "No patrol tiles to clear";
                    }
                    else
                    {
                        Handheld::StaffPatrol::Configure(choice);
                        invalidate();
                    }
                    break;
                case Page::staffPatrolClear:
                    if (choice == 1)
                        page(Page::staffPatrolOptions);
                    else if (_patrolDraft)
                    {
                        auto result = Handheld::StaffPatrol::Clear(*_patrolDraft, true);
                        if (result.error == GameActions::Status::ok)
                            page(Page::staffPatrolOptions);
                        else
                        {
                            _status = result.getErrorMessage();
                            invalidate();
                        }
                    }
                    break;
                case Page::rightsOptions:
                    if (choice == 0)
                        close();
                    else
                    {
                        Handheld::RightsTools::Configure(choice);
                        invalidate();
                    }
                    break;
                case Page::rightsConfirm:
                    if (choice == 1)
                    {
                        _rightsDraft.reset();
                        close();
                    }
                    else if (_rightsDraft)
                    {
                        const auto r = Handheld::RightsTools::Action(*_rightsDraft, true);
                        if (r.error == GameActions::Status::ok)
                        {
                            _rightsDraft.reset();
                            close();
                        }
                        else
                        {
                            _status = r.getErrorMessage();
                            invalidate();
                        }
                    }
                    break;
                case Page::areaOptions:
                    if (choice == 0)
                        close();
                    else if (!Handheld::ConstructionAdjust::TerrainEnabled(choice))
                    {
                        _status = "Fixed or unavailable for this tool";
                        invalidate();
                    }
                    else if (choice == 5 || choice == 6)
                        openTerrainMaterialPicker(choice == 6, true);
                    else
                    {
                        Handheld::AreaTools::Configure(choice);
                        invalidate();
                    }
                    break;
                case Page::build:
                    for (const auto& card : choices())
                        if (card.action == choice)
                        {
                            sLastBuildCard = choice;
                            sLastBuildLabel = card.label;
                            sLastBuildSprite = card.sprite;
                        }
                    if (choice == 12)
                    {
                        close();
                        Handheld::BeginRightsTools();
                    }
                    else if (choice == 11)
                    {
                        close();
                        Handheld::BeginAreaTools();
                    }
                    else if (choice == 10)
                        page(Page::sceneryCategory);
                    else if (choice == 9)
                        openDesignPicker(true);
                    else if (choice == 8)
                        openRidePicker(true);
                    else if (choice == 7)
                        openShopPicker(true);
                    else if (choice == 2)
                        page(Page::paths);
                    else
                    {
                        close();
                        if (choice < 2)
                            Handheld::BeginTerrain(choice == 0);
                        else if (choice == 3)
                            Handheld::BeginWater();
                        else if (choice == 4)
                            Handheld::BeginSurface();
                        else if (choice == 5)
                            Handheld::BeginSceneryRemoval();
                        else if (choice == 6)
                            Handheld::BeginTrees();
                    }
                    break;
                case Page::paths:
                    if (choice == 3)
                        page(Page::build);
                    else
                    {
                        close();
                        Handheld::EndTerrain();
                        if (choice >= 4)
                            Handheld::Paths::BeginConnected(choice == 5);
                        else
                            Handheld::Paths::Begin(choice == 1, choice == 2);
                    }
                    break;
                case Page::pathUndoConfirm:
                    if (choice == 1)
                    {
                        _pathUndo.reset();
                        page(Page::pathOptions);
                    }
                    else if (_pathUndo)
                    {
                        const auto result = Handheld::Paths::Undo(*_pathUndo, true);
                        if (result.error == GameActions::Status::ok)
                        {
                            _pathUndo.reset();
                            close();
                        }
                        else
                        {
                            _status = result.getErrorMessage();
                            invalidate();
                        }
                    }
                    break;
                case Page::pathOptions:
                {
                    namespace Paths = Handheld::Paths;
                    bool changed = true;
                    if (Paths::IsConnected())
                    {
                        if (choice == 0)
                            close();
                        else if (choice == 1)
                            Paths::TurnConnected();
                        else if (choice == 2 || choice == 3)
                            Paths::SlopeConnected(choice == 2 ? 1 : -1);
                        else if (choice == 4)
                            Paths::LevelConnected();
                        else if (choice == 5)
                            Paths::BeginConnected(Paths::IsQueue());
                        else if (choice == 6)
                            openPathSurfacePicker();
                        else if (choice == 7)
                        {
                            if (_pausedHere && (gGamePaused & GAME_PAUSED_NORMAL))
                            {
                                PauseToggle();
                                _pathUndoResumed = true;
                            }
                            _pathUndo = Paths::BeginUndo();
                            if (_pathUndo)
                            {
                                page(Page::pathUndoConfirm);
                                _focus.move(1, 2);
                            }
                            else
                            {
                                if (_pathUndoResumed && _pausedHere && !(gGamePaused & GAME_PAUSED_NORMAL))
                                    PauseToggle();
                                _pathUndoResumed = false;
                                _status = "No unchanged new section to undo";
                            }
                        }
                        invalidate();
                        break;
                    }
                    switch (choice)
                    {
                        case 0:
                            close();
                            return;
                        case 1:
                            Paths::FollowGround();
                            break;
                        case 2:
                        case 3:
                            changed = Paths::ChangeHeight(choice == 2 ? 1 : -1);
                            break;
                        case 4:
                            changed = Paths::CycleSlope();
                            break;
                        case 5:
                            changed = Paths::RotateSlope();
                            break;
                        case 6:
                            openPathSurfacePicker();
                            break;
                    }
                    _status = changed ? Paths::PlacementName() : "No target or height limit reached.";
                    invalidate();
                    break;
                }
                case Page::dashboard:
                    if (_dashboard.category().available)
                        enterDashboardContent();
                    else
                    {
                        _status = "Section not implemented";
                        invalidate();
                    }
                    break;
                case Page::staff:
                    _staffRosterFocus = choice;
                    if (choice == 0)
                        page(Page::staffRoles);
                    else if (choice >= 3 && choice - 3 < _staffRoster.size())
                    {
                        _staffTarget = _staffRoster[choice - 3];
                        _staffTabs = {};
                        _staffFromPark = false;
                        _ordersBaseline = _ordersDraft = Handheld::StaffManagement::Resolve(*_staffTarget)->staffOrders;
                        page(Page::staffInspect);
                    }
                    else if (choice == 1)
                    {
                        _staffFilter = !_staffFilter ? std::optional<StaffType>(StaffType::handyman)
                            : *_staffFilter == StaffType::entertainer
                            ? std::nullopt
                            : std::optional<StaffType>(static_cast<StaffType>(EnumValue(*_staffFilter) + 1));
                        _staffRosterFocus = 0;
                        page(Page::staff);
                    }
                    else if (choice == 2)
                        page(Page::staffSummary);
                    else
                        page(Page::dashboard);
                    break;
                case Page::staffRoles:
                    if (choice < 4)
                    {
                        _hireType = static_cast<StaffType>(choice);
                        page(Page::staffHire);
                    }
                    else if (choice == 5)
                        _hireManual = !_hireManual;
                    else
                        page(Page::staff);
                    break;
                case Page::staffCostumes:
                    if (_staffTarget)
                    {
                        const auto entries = Handheld::StaffManagement::Costumes();
                        if (choice >= entries.size())
                            page(Page::staffInspect);
                        else
                        {
                            const auto result = Handheld::StaffManagement::SetCostume(
                                *_staffTarget, _costumeBaseline, entries[choice].first, true);
                            if (result.error == GameActions::Status::ok)
                                page(Page::staffInspect);
                            else
                                _status = Handheld::Finances::Feedback(result);
                        }
                    }
                    break;
                case Page::staffHire:
                    if (choice == 1)
                        page(Page::staff);
                    else if (!_staffBusy)
                    {
                        _staffBusy = true;
                        auto result = Handheld::StaffManagement::Hire(
                            _hireType, true,
                            [this, lifetime = std::weak_ptr<int>(_staffLifetime),
                             request = _staffRequest](const GameActions::GameAction*, const GameActions::Result* response) {
                                if (lifetime.lock() && request == _staffRequest)
                                    staffHired(*response);
                            },
                            _hireManual);
                        if (result.error != GameActions::Status::ok)
                        {
                            _staffBusy = false;
                            _status = Handheld::Finances::Feedback(result);
                        }
                        else
                            _status = "Hiring...";
                    }
                    break;
                case Page::staffInspect:
                    if (!_staffTarget || !Handheld::StaffManagement::Resolve(*_staffTarget))
                    {
                        page(Page::staff);
                        break;
                    }
                    if (choice == 0)
                    {
                        const auto target = *_staffTarget;
                        if (!Handheld::StaffManagement::Resolve(target)->getLocation().isNull() && WindowGetMain())
                        {
                            close();
                            Handheld::StaffManagement::Locate(target);
                            return;
                        }
                        _status = "Location unavailable";
                    }
                    else if (choice == 9)
                    {
                        const auto person = *_staffTarget;
                        if (!Handheld::StaffManagement::Resolve(person)->canBePickedUp())
                        {
                            _status = "Person cannot be picked up now";
                            break;
                        }
                        saveStaffReturn();
                        close();
                        if (!Handheld::BeginPeepPlacement({ {}, person }))
                            HandheldStaffOpen(person);
                        return;
                    }
                    else if (choice == 8)
                    {
                        if (_staffTarget->type == StaffType::entertainer)
                            _status = "Entertainers use costume colours";
                        else
                        {
                            _staffUniformBaseline = _staffUniformDraft = StaffGetColour(_staffTarget->type);
                            page(Page::staffUniform);
                        }
                    }
                    else if (choice == 7)
                    {
                        _staffNameBaseline = Handheld::StaffManagement::Resolve(*_staffTarget)->getName();
                        _nameEntry = { _staffNameBaseline };
                        page(Page::staffName);
                    }
                    else if (choice == 6)
                    {
                        const auto employee = *_staffTarget;
                        saveStaffReturn();
                        close();
                        if (!Handheld::BeginStaffPatrol(employee))
                            HandheldStaffOpen(employee);
                        return;
                    }
                    else if (choice == 4)
                    {
                        _costumeBaseline = Handheld::StaffManagement::Resolve(*_staffTarget)->animationObjectIndex;
                        page(Page::staffCostumes);
                    }
                    else if (choice == 1)
                    {
                        if (Handheld::StaffManagement::Orders(_staffTarget->type).empty())
                        {
                            _status = "This role has no adjustable duties";
                            break;
                        }
                        _ordersBaseline = _ordersDraft = Handheld::StaffManagement::Resolve(*_staffTarget)->staffOrders;
                        page(Page::staffOrders);
                    }
                    else
                        page(choice == 2 ? Page::staffStats : choice == 3 ? Page::staffDismiss : Page::staff);
                    break;
                case Page::staffSummary:
                    page(Page::staff);
                    break;
                case Page::staffUniform:
                    if (_staffTarget)
                    {
                        const auto result = Handheld::StaffManagement::SetUniform(
                            *_staffTarget, _staffUniformBaseline, _staffUniformDraft, true);
                        if (result.error == GameActions::Status::ok)
                            page(Page::staffInspect);
                        else
                        {
                            _status = Handheld::Finances::Feedback(result);
                            invalidate();
                        }
                    }
                    break;
                case Page::staffName:
                    if (choice == 47)
                        page(Page::staffInspect);
                    else if (choice == 46 && _staffTarget)
                    {
                        const auto result = Handheld::StaffManagement::SetName(
                            *_staffTarget, _staffNameBaseline, _nameEntry.text, true);
                        if (result.error == GameActions::Status::ok)
                            page(Page::staffInspect);
                        else
                            _status = Handheld::Finances::Feedback(result);
                    }
                    else
                        _nameEntry.Press(choice);
                    break;
                case Page::staffNameConfirm:
                    if (choice == 1)
                        page(Page::staffName);
                    else if (_staffTarget)
                    {
                        const auto result = Handheld::StaffManagement::SetName(
                            *_staffTarget, _staffNameBaseline, _nameEntry.text, true);
                        if (result.error == GameActions::Status::ok)
                            page(Page::staffInspect);
                        else
                            _status = Handheld::Finances::Feedback(result);
                    }
                    break;
                case Page::staffStats:
                    page(Page::staffInspect);
                    break;
                case Page::staffOrders:
                    if (!_staffTarget || !Handheld::StaffManagement::Resolve(*_staffTarget))
                    {
                        page(Page::staff);
                        break;
                    }
                    {
                        const auto orders = Handheld::StaffManagement::Orders(_staffTarget->type);
                        if (choice < orders.size())
                            _ordersDraft ^= orders[choice].first;
                        else if (choice == orders.size())
                        {
                            const auto result = Handheld::StaffManagement::SetOrders(
                                *_staffTarget, _ordersBaseline, _ordersDraft, true);
                            if (result.error == GameActions::Status::ok)
                            {
                                _ordersBaseline = _ordersDraft;
                                page(Page::staffInspect);
                                _status = "Orders submitted";
                            }
                            else
                                _status = Handheld::Finances::Feedback(result);
                        }
                        else
                        {
                            _ordersBaseline = _ordersDraft = Handheld::StaffManagement::Resolve(*_staffTarget)->staffOrders;
                            page(Page::staffInspect);
                        }
                    }
                    break;
                case Page::staffDismiss:
                    if (choice == 1)
                        page(Page::staffInspect);
                    else if (_staffTarget)
                    {
                        const auto result = Handheld::StaffManagement::Dismiss(*_staffTarget, true);
                        if (result.error == GameActions::Status::ok)
                            page(Page::staff);
                        else
                            _status = Handheld::Finances::Feedback(result);
                    }
                    break;
                case Page::financeHistory:
                    if (choice == 0)
                        _financeGraph = (_financeGraph + 1) % 3;
                    else if (choice == 1)
                        _financeSample = std::min<size_t>(63, _financeSample + 1);
                    else if (choice == 2 && _financeSample)
                        --_financeSample;
                    else if (choice == 3)
                        page(Page::finances);
                    invalidate();
                    break;
                case Page::financeLedger:
                    if (choice == 14)
                        _ledger.Move(-1);
                    else if (choice == 15)
                        _ledger.Move(1);
                    else if (choice == 16)
                        page(Page::finances);
                    invalidate();
                    break;
                case Page::finances:
                    if (_financeTabs.tab == 1)
                        break;
                    if (_financeTabs.tab == 2)
                    {
                        _financeGraph = (_financeGraph + 1) % 3;
                        invalidate();
                        break;
                    }
                    if (_financeTabs.tab == 0 && choice > 0)
                    {
                        financeTab(choice);
                        break;
                    }
                    {
                        const auto field = choice == 0 ? Handheld::Finances::Field::admission : Handheld::Finances::Field::loan;
                        if (Handheld::Finances::Available(field))
                        {
                            _financeReturnPage = Page::finances;
                            _financeDraft = Handheld::Finances::Begin(field);
                            page(Page::financeEditor);
                        }
                    }
                    break;
                case Page::financeEditor:
                    if (choice == 1)
                        page(_financeReturnPage);
                    else if (_financeDraft)
                    {
                        const auto result = Handheld::Finances::Action(*_financeDraft, false);
                        if (result.error == GameActions::Status::ok)
                        {
                            if (_financeDraft->field == Handheld::Finances::Field::loan)
                                page(Page::financeConfirm);
                            else
                                applyFinanceDraft();
                        }
                        else
                        {
                            _status = Handheld::Finances::Feedback(result);
                            invalidate();
                        }
                    }
                    break;
                case Page::financeConfirm:
                    if (choice == 1)
                        page(_financeReturnPage);
                    else
                        applyFinanceDraft();
                    break;
                case Page::mapSubjects:
                    if (choice < _mapSubjects.size())
                    {
                        _mapTarget = _mapSubjects[choice];
                        _mapSubjectsFocus = choice;
                        page(Page::mapSubject);
                    }
                    else if (choice == _mapSubjects.size() && _parkMap.mode >= 2)
                    {
                        _parkMap.filter = (_parkMap.filter + 1) % (_parkMap.mode == 2 ? 2 : 5);
                        _mapSubjects = Handheld::MapSubjects::Roster(_parkMap.mode, _parkMap.filter, _parkMap.guestFilter);
                        _focus = {};
                        invalidate();
                    }
                    else
                        page(Page::parkMap);
                    break;
                case Page::mapSubject:
                    if (choice == 1 && _mapTarget)
                        inspectMapSubject(*_mapTarget, Page::mapSubject);
                    else if ((choice == 0 || choice == 2) && _mapTarget)
                    {
                        const auto location = Handheld::MapSubjects::Location(*_mapTarget);
                        if (location && MapIsLocationValid(*location))
                        {
                            if (choice == 0)
                            {
                                _parkMap.subject = *_mapTarget;
                                _parkMap.focus = *location;
                                _parkMap.Clamp();
                                page(Page::parkMap);
                            }
                            else if (auto* main = WindowGetMain())
                            {
                                close();
                                WindowScrollToLocation(*main, *location);
                            }
                        }
                        else
                        {
                            _status = "Subject departed or location unavailable";
                            invalidate();
                        }
                    }
                    else
                        back();
                    break;
                case Page::parkNews:
                    if (choice < _parkAlerts.size())
                    {
                        const auto index = choice;
                        close();
                        Handheld::JumpToAlert(_parkAlerts[index]);
                        break;
                    }
                    choice -= _parkAlerts.size();
                    if (choice < _parkNews.size())
                    {
                        _parkNewsTarget = _parkNews[choice];
                        _parkNewsFocus = choice + _parkAlerts.size();
                        _parkNewsPage = 0;
                        page(Page::parkNewsDetail);
                    }
                    else
                    {
                        _parkNews = Handheld::ParkInformation::NewsList();
                        invalidate();
                    }
                    break;
                case Page::parkNewsDetail:
                    if (choice >= newsActions().size())
                        break;
                    choice = newsActions()[choice];
                    if (choice == 0 && _parkNewsTarget)
                    {
                        const auto count = Handheld::GuestManagement::TextLines(
                                               Handheld::ParkInformation::PlainText(_parkNewsTarget->item.text))
                                               .size();
                        _parkNewsPage = (_parkNewsPage + 1) % std::max<size_t>(1, (count + 5) / 6);
                        invalidate();
                    }
                    else if (choice == 1 && _parkNewsTarget)
                    {
                        const auto location = Handheld::ParkInformation::Location(*_parkNewsTarget);
                        if (location && WindowGetMain())
                        {
                            auto* main = WindowGetMain();
                            close();
                            WindowScrollToLocation(*main, *location);
                        }
                        else
                        {
                            _status = "News expired or subject unavailable";
                            invalidate();
                        }
                    }
                    else if (choice == 2 && _parkNewsTarget)
                    {
                        const auto link = Handheld::ManagementNews::Available(*_parkNewsTarget);
                        if (link != Handheld::ManagementNews::Link::none)
                        {
                            if (link == Handheld::ManagementNews::Link::finance)
                            {
                                _dashboard.section = Handheld::Dashboard::Section::finances;
                                page(Page::finances);
                            }
                            else if (link == Handheld::ManagementNews::Link::marketing)
                            {
                                _dashboard.section = Handheld::Dashboard::Section::marketing;
                                page(Page::marketing);
                            }
                            else if (link == Handheld::ManagementNews::Link::guests)
                            {
                                _dashboard.section = Handheld::Dashboard::Section::guests;
                                page(Page::guestRoster);
                            }
                            else if (link == Handheld::ManagementNews::Link::park)
                                page(Page::overview);
                            else if (link == Handheld::ManagementNews::Link::awards)
                                page(Page::parkAwards);
                            else
                            {
                                _researchItem = Handheld::ManagementNews::ResearchTarget(*_parkNewsTarget);
                                _researchCatalog = Handheld::ResearchCatalog::Roster(0);
                                _researchFilter = 0;
                                page(Page::researchItem);
                            }
                            _managementNewsReturn = true;
                        }
                        else if (!Handheld::ParkInformation::SubjectCurrent(*_parkNewsTarget))
                        {
                            _status = "News subject unavailable";
                            invalidate();
                        }
                        else if (_parkNewsTarget->ride)
                            inspectMapSubject(*_parkNewsTarget->ride, Page::parkNewsDetail);
                        else if (_parkNewsTarget->guest)
                            inspectMapSubject(*_parkNewsTarget->guest, Page::parkNewsDetail);
                        else if (_parkNewsTarget->staff)
                            inspectMapSubject(*_parkNewsTarget->staff, Page::parkNewsDetail);
                    }
                    else if (
                        choice == 4 && _parkNewsTarget && Handheld::ParkInformation::DismissCurrentMessage(*_parkNewsTarget))
                    {
                        _parkNews = Handheld::ParkInformation::NewsList();
                        page(Page::parkNews);
                    }
                    else if (choice == 3)
                    {
                        page(Page::parkNews);
                        _focus.move(static_cast<int>(_parkNewsFocus), rows().size());
                    }
                    break;
                case Page::parkAwards:
                    if (choice + 1 == rows().size())
                        page(Page::overview);
                    break;
                case Page::parkHistory:
                    if (choice == 0)
                    {
                        _parkGuestHistory = !_parkGuestHistory;
                        invalidate();
                    }
                    else
                        page(Page::overview);
                    break;
                case Page::parkMap:
                    if (auto* main = WindowGetMain())
                    {
                        const auto location = _parkMap.focus;
                        close();
                        WindowScrollToLocation(*main, CoordsXYZ{ location, MapGetHighestZ(location) });
                    }
                    break;
                case Page::overview:
                    if (_parkTabs.tab == 0 && choice == 3)
                    {
                        _parkTabs.save(_focus.index());
                        _parkTabs.tab = 1;
                        _focus = {};
                        invalidate();
                        break;
                    }
                    if (_parkTabs.tab == 1)
                        choice = 2;
                    else if (_parkTabs.tab == 2)
                        choice += 3;
                    if (choice >= 3 && choice <= 5)
                    {
                        Handheld::EndTerrain();
                        _newsFromPark = false;
                        _newsFromDashboard = false;
                        _parkNews = Handheld::ParkInformation::NewsList();
                        page(choice == 3 ? Page::parkNews : choice == 4 ? Page::parkAwards : Page::parkHistory);
                    }
                    else if (choice == 2)
                    {
                        Handheld::EndTerrain();
                        _parkMap.Begin();
                        page(Page::parkMap);
                    }
                    else if (choice == 0)
                    {
                        // Opening and closing the park is free and reversible: change it in place.
                        const bool open = getGameState().park.flags.has(ParkFlag::parkOpen);
                        setParkOpen(!open);
                    }
                    else if (Handheld::Finances::Available(Handheld::Finances::Field::admission))
                    {
                        _financeReturnPage = Page::overview;
                        _financeDraft = Handheld::Finances::Begin(Handheld::Finances::Field::admission);
                        page(Page::financeEditor);
                    }
                    else
                    {
                        _status = "Admission pricing unavailable";
                        invalidate();
                    }
                    break;
                case Page::overviewConfirm:
                    if (choice == 1)
                    {
                        page(Page::overview);
                        break;
                    }
                    if (_overviewDraft)
                    {
                        const auto query = Handheld::ParkOverview::Action(*_overviewDraft, false);
                        if (query.error != GameActions::Status::ok)
                        {
                            _status = Handheld::Finances::Feedback(query);
                            invalidate();
                            break;
                        }
                        _overviewBusy = true;
                        const auto request = ++_overviewRequest;
                        const auto result = Handheld::ParkOverview::Action(
                            *_overviewDraft, true,
                            [this, request, lifetime = std::weak_ptr<int>(_overviewLifetime)](
                                const GameActions::GameAction*, const GameActions::Result* outcome) {
                                if (lifetime.expired() || request != _overviewRequest || _page != Page::overviewConfirm)
                                    return;
                                _overviewBusy = false;
                                if (outcome->error == GameActions::Status::ok)
                                {
                                    page(Page::overview);
                                    _status = "Park status updated";
                                }
                                else
                                    _status = Handheld::Finances::Feedback(*outcome);
                                invalidate();
                            });
                        if (result.error != GameActions::Status::ok)
                        {
                            _overviewBusy = false;
                            _status = Handheld::Finances::Feedback(result);
                        }
                        invalidate();
                    }
                    break;
                case Page::ridesRoster:
                    _rideFromPark = false;
                    _ridesRosterFocus = choice;
                    _ridesInspectFocus = 0;
                    _rideTabs = {};
                    _ridesTabChild = false;
                    if (choice >= _ridesRoster.size())
                        page(Page::dashboard);
                    else if (Handheld::RideManagement::Resolve(_ridesRoster[choice]))
                    {
                        _ridesTarget = _ridesRoster[choice];
                        page(Page::ridesInspect);
                    }
                    else
                    {
                        _status = "Ride unavailable";
                        invalidate();
                    }
                    break;
                case Page::trackBlocks:
                    if (choice < _trackBlocks.size())
                    {
                        _trackBlock = _trackBlocks[choice];
                        _trackSpeed = _trackBlock->speed;
                        page(Page::trackBlock);
                    }
                    else
                        page(Page::ridesInspect);
                    break;
                case Page::trackContext:
                    if (!_trackBlock || !Handheld::TrackEditing::Valid(*_trackBlock) || choice + 1 == rows().size())
                        page(Page::trackBlock);
                    else if (choice + 2 == rows().size())
                    {
                        _trackTechnical = !_trackTechnical;
                        const auto count = rows().size();
                        _focus.move(static_cast<int>(count - 2) - static_cast<int>(_focus.index()), count);
                        invalidate();
                    }
                    break;
                case Page::trackBlock:
                    if (!_trackBlock || !Handheld::TrackEditing::Valid(*_trackBlock))
                    {
                        if (_ridesTarget)
                            _trackBlocks = Handheld::TrackEditing::Blocks(*_ridesTarget);
                        page(Page::trackBlocks);
                        _status = "Block changed; select again";
                    }
                    else if (ridesPresentationFor(Page::trackBlock, choice).rows[choice] == "Inspect connections / station...")
                        page(Page::trackContext);
                    else if (ridesPresentationFor(Page::trackBlock, choice).rows[choice] == "Continue section...")
                    {
                        const auto block = *_trackBlock;
                        launchRideTool([=] { return Handheld::ResumeCustomTrackFrom(block.ride, block.origin, block.type); });
                        if (!Handheld::CustomTracks::Active())
                            GetWindowManager()->ShowError(
                                "Cannot continue section", "No compatible forward endpoint; select again");
                    }
                    else if (ridesPresentationFor(Page::trackBlock, choice).rows[choice] == "Build backward...")
                    {
                        const auto block = *_trackBlock;
                        launchRideTool(
                            [=] { return Handheld::ResumeCustomTrackFrom(block.ride, block.origin, block.type, true); });
                        if (!Handheld::CustomTracks::Active())
                            GetWindowManager()->ShowError(
                                "Cannot build backward", "No compatible backward endpoint; select again");
                    }
                    else if (ridesPresentationFor(Page::trackBlock, choice).rows[choice] == "Replace block...")
                    {
                        _trackPrevious = Handheld::TrackEditing::Previous(*_trackBlock);
                        const auto result = Handheld::TrackEditing::RemoveForReplacement(*_trackBlock);
                        if (result.error == GameActions::Status::ok)
                        {
                            _trackCost = result.cost;
                            _trackContext = Handheld::TrackEditing::Context(*_trackBlock);
                            page(Page::trackReplaceConfirm);
                        }
                        else
                        {
                            _status = result.getErrorMessage();
                            invalidate();
                        }
                    }
                    else if (ridesPresentationFor(Page::trackBlock, choice).rows[choice] == "Locate block")
                    {
                        const auto origin = _trackBlock->origin;
                        close();
                        if (auto* main = WindowGetMain())
                            WindowScrollToLocation(*main, origin);
                    }
                    else if (
                        ridesPresentationFor(Page::trackBlock, choice).rows[choice] == "Remove block..."
                        || ridesPresentationFor(Page::trackBlock, choice).rows[choice] == "Review speed...")
                    {
                        const auto speed = ridesPresentationFor(Page::trackBlock, choice).rows[choice] == "Review speed..."
                            ? std::optional<uint8_t>{ _trackSpeed }
                            : std::nullopt;
                        const auto result = Handheld::TrackEditing::Change(*_trackBlock, speed);
                        if (result.error == GameActions::Status::ok)
                        {
                            _trackCost = result.cost;
                            _trackContext = Handheld::TrackEditing::Context(*_trackBlock);
                            page(
                                ridesPresentationFor(Page::trackBlock, choice).rows[choice] == "Remove block..."
                                    ? Page::trackRemoveConfirm
                                    : Page::trackSpeedConfirm);
                        }
                        else
                        {
                            _status = result.getErrorMessage();
                            invalidate();
                        }
                    }
                    else if (ridesPresentationFor(Page::trackBlock, choice).rows[choice] == "Back to blocks")
                    {
                        _trackBlocks = Handheld::TrackEditing::Blocks(*_ridesTarget);
                        page(Page::trackBlocks);
                    }
                    else if (trackTypeHasSpeedSetting(_trackBlock->type))
                    {
                        const int minimum = getGameState().cheats.unlockOperatingLimits ? 0 : 2;
                        _trackSpeed = std::clamp<int>(
                            _trackSpeed + (ridesPresentationFor(Page::trackBlock, choice).rows[choice] == "Speed -" ? -2 : 2),
                            minimum, kMaximumTrackSpeed);
                        invalidate();
                    }
                    break;
                case Page::trackReplaceConfirm:
                    if (choice == 0)
                        page(Page::trackBlock);
                    else if (_trackBlock)
                    {
                        const auto block = *_trackBlock;
                        const auto cost = _trackCost;
                        const auto context = _trackContext;
                        const auto navigation = rideContext();
                        close();
                        const auto result = Handheld::TrackEditing::RemoveForReplacement(
                            block, cost, true,
                            [block, navigation](const auto*, const GameActions::Result* outcome) {
                                if (outcome->error != GameActions::Status::ok)
                                {
                                    HandheldRideOpen(block.ride.id);
                                    GetWindowManager()->ShowError("Cannot remove for rebuild", outcome->getErrorMessage());
                                    return;
                                }
                                if (!Handheld::ResumeCustomTrackReplacement(
                                        block.ride,
                                        { block.origin, block.type, block.speed, block.seat, block.chain, block.inverted },
                                        block.colour))
                                {
                                    HandheldRideOpen(block.ride.id);
                                    GetWindowManager()->ShowError(
                                        "Track removed", "Continue building, then check entrances and exits");
                                }
                                else
                                    Handheld::RideNavigation::construction = navigation;
                            },
                            context);
                        if (result.error != GameActions::Status::ok)
                        {
                            HandheldRideOpen(block.ride.id);
                            GetWindowManager()->ShowError("Cannot remove for rebuild", result.getErrorMessage());
                        }
                    }
                    break;
                case Page::trackRemoveConfirm:
                case Page::trackSpeedConfirm:
                    if (choice == 0)
                        page(Page::trackBlock);
                    else if (_trackBlock)
                    {
                        const auto block = *_trackBlock;
                        const auto speed = _page == Page::trackSpeedConfirm ? std::optional<uint8_t>{ _trackSpeed }
                                                                            : std::nullopt;
                        const auto cost = _trackCost;
                        const auto context = _trackContext;
                        close();
                        const auto result = Handheld::TrackEditing::Change(block, speed, cost, true, context);
                        if (result.error != GameActions::Status::ok)
                            GetWindowManager()->ShowError("Cannot edit track", result.getErrorMessage());
                    }
                    break;
                case Page::ridesInspect:
                {
                    if (!_ridesTarget || !Handheld::RideManagement::Resolve(*_ridesTarget))
                    {
                        page(Page::ridesRoster);
                        break;
                    }
                    _rideTabs.save(_focus.index());
                    if (_rideTabs.tab == Handheld::RideTabs::statistics)
                    {
                        _ridesTabChild = true;
                        if (choice == 2)
                            Handheld::RideManagement::BeginGraphs(*_ridesTarget);
                        page(choice == 0 ? Page::ridesCustomers : choice == 1 ? Page::ridesEarnings : Page::ridesGraphs);
                        break;
                    }
                    if (_rideTabs.tab == Handheld::RideTabs::overview)
                    {
                        // Row 0 is the state switch: A steps to the next state, wrapping round.
                        if (choice == 0)
                        {
                            stepRideState(1, true);
                            break;
                        }
                        --choice;
                    }
                    auto nativeModel = Handheld::RideManagement::Presentation(
                        Handheld::RideManagement::View::inspection, 0, _ridesRoster, _ridesTarget);
                    const auto indices = Handheld::RideTabs::Indices(nativeModel, _rideTabs.tab);
                    if (choice >= indices.size())
                        break;
                    choice = indices[choice];
                    auto rows = nativeModel.rows;
                    if (const auto along = std::find(rows.begin(), rows.end(), "Ride along"); along != rows.end())
                    {
                        const auto index = static_cast<size_t>(along - rows.begin());
                        if (choice == index)
                        {
                            const auto ride = _ridesTarget->id;
                            Handheld::GuestFollow::Stop();
                            close();
                            if (!Handheld::RideFollow::Start(ride))
                                GetWindowManager()->ShowError("Ride along", "This ride has no trains yet");
                            break;
                        }
                        rows.erase(along);
                        if (choice > index)
                            --choice;
                    }
                    const auto accessRow = std::find(rows.begin(), rows.end(), "Entrances / exits...");
                    if (accessRow != rows.end())
                    {
                        const auto index = static_cast<size_t>(accessRow - rows.begin());
                        if (choice == index)
                        {
                            openAccessManagement();
                            break;
                        }
                        rows.erase(accessRow);
                        if (choice > index)
                            --choice;
                    }
                    // Keep the placement handoff first without changing the established action indices.
                    if (Handheld::FlatRides::CanResume(*_ridesTarget))
                    {
                        if (choice == 0)
                        {
                            const auto target = *_ridesTarget;
                            launchRideTool([=] { return Handheld::ResumeFlatRide(target.id); });
                            Handheld::RideManagement::Locate(target);
                            break;
                        }
                        // Reconstruct the old ordering for the existing dispatcher below.
                        rows.erase(rows.begin());
                        rows.insert(rows.begin() + 7, "Place entrance / exit...");
                        if (choice <= 7)
                            --choice;
                    }
                    if (choice < rows.size() && rows[choice] == "Edit track...")
                    {
                        if (_pausedHere && (gGamePaused & GAME_PAUSED_NORMAL))
                        {
                            PauseToggle();
                            _pausedHere = false;
                        }
                        _trackBlocks = Handheld::TrackEditing::Blocks(*_ridesTarget);
                        _trackBlock.reset();
                        _dashboard.section = Handheld::Dashboard::Section::rides;
                        _dashboard.focus = Handheld::Dashboard::Focus::content;
                        page(Page::trackBlocks);
                        break;
                    }

                    if (choice < rows.size() && rows[choice] == "Save design...")
                    {
                        const auto result = Handheld::DesignAuthoring::Begin(*_ridesTarget);
                        if (result.error == GameActions::Status::ok)
                        {
                            _nameEntry = { Handheld::DesignAuthoring::DefaultName() };
                            page(Page::designScenery);
                        }
                        else
                            _status = result.getErrorMessage();
                        invalidate();
                        break;
                    }
                    if (choice == rows.size() - 8)
                    {
                        _ridesDraft = Handheld::RideManagement::BeginName(*_ridesTarget);
                        if (_ridesDraft)
                        {
                            _nameEntry = { _ridesDraft->name };
                            page(Page::ridesName);
                        }
                        break;
                    }
                    if (choice == rows.size() - 7)
                    {
                        page(Page::ridesAppearance);
                        break;
                    }
                    if (choice == rows.size() - 6)
                    {
                        page(Page::ridesMusic);
                        break;
                    }
                    if (choice == rows.size() - 5)
                    {
                        page(Page::ridesVehicles);
                        break;
                    }
                    if (choice == rows.size() - 4)
                    {
                        page(Page::ridesDispatch);
                        break;
                    }
                    if (choice == rows.size() - 3)
                    {
                        page(Page::ridesOperations);
                        break;
                    }
                    if (choice == rows.size() - 2)
                    {
                        page(Page::ridesMaintenance);
                        break;
                    }
                    if (choice == rows.size() - 1)
                    {
                        _ridesDraft = Handheld::RideManagement::BeginDemolition(*_ridesTarget);
                        if (_ridesDraft)
                        {
                            if (_pausedHere && (gGamePaused & GAME_PAUSED_NORMAL))
                            {
                                PauseToggle();
                                _ridesReviewResumed = true;
                            }
                            page(Page::ridesConfirm);
                        }
                        break;
                    }
                    if (choice < rows.size() && rows[choice] == "Edit maze...")
                    {
                        const auto t = *_ridesTarget;
                        launchRideTool([=] { return Handheld::ResumeMaze(t.id); });
                        break;
                    }
                    if (choice == 0)
                    {
                        const auto target = *_ridesTarget;
                        if (WindowGetMain() && Handheld::RideManagement::Location(target))
                        {
                            close();
                            Handheld::RideManagement::Locate(target);
                        }
                        else
                        {
                            _status = "Location unavailable";
                            invalidate();
                        }
                    }
                    else if (choice == 1)
                        page(Page::ridesStats);
                    else if (choice == 6)
                        page(Page::ridesRoster);
                    else if (
                        choice == (Handheld::FlatRides::CanResume(*_ridesTarget) ? 8u : 7u)
                        && Handheld::RideAccess::Supported(*_ridesTarget))
                        page(Page::ridesAccess);
                    else if (
                        Handheld::CustomTracks::CanResume(*_ridesTarget)
                        && choice
                            == 7u + (Handheld::FlatRides::CanResume(*_ridesTarget) ? 1u : 0u)
                                + (Handheld::RideAccess::Supported(*_ridesTarget) ? 1u : 0u))
                    {
                        const auto target = *_ridesTarget;
                        launchRideTool([=] { return Handheld::ResumeCustomTrack(target.id); });
                    }
                    else if (choice == 7 && Handheld::FlatRides::CanResume(*_ridesTarget))
                    {
                        const auto target = *_ridesTarget;
                        launchRideTool([=] { return Handheld::ResumeFlatRide(target.id); });
                        Handheld::RideManagement::Locate(target);
                    }
                    else
                    {
                        const auto status = choice == 2 ? std::optional<RideStatus>{ RideStatus::open }
                            : choice == 3               ? std::optional<RideStatus>{ RideStatus::closed }
                            : choice == 4               ? std::optional<RideStatus>{ RideStatus::testing }
                                                        : std::nullopt;
                        _ridesDraft = Handheld::RideManagement::Begin(*_ridesTarget, status);
                        if (_ridesDraft)
                            page(status ? Page::ridesConfirm : Page::ridesPrice);
                        else
                        {
                            _status = "Action unavailable for this ride";
                            invalidate();
                        }
                    }
                    break;
                }
                case Page::accessManage:
                    if (choice < _accessPoints.size())
                    {
                        _accessFocus = choice;
                        _accessPoint = _accessPoints[choice];
                        page(Page::accessPoint);
                    }
                    else
                        page(Page::ridesInspect);
                    break;
                case Page::accessPoint:
                    if (choice == 2 && canConnectAccessPoint())
                    {
                        const auto point = *_accessPoint;
                        // BeginPaths revalidates native access and locates its outside landing.
                        // Retain this page on a failed handoff instead of silently closing it.
                        const auto context = rideContext();
                        if (Handheld::RideAccess::BeginPaths(point.target, point.exit, true))
                        {
                            Handheld::RideNavigation::construction = context;
                            close();
                        }
                        else
                        {
                            _status = "Cannot start path here; return to ride inspection";
                            invalidate();
                        }
                    }
                    else if (!_accessPoint || !Handheld::RideAccess::Current(*_accessPoint) || choice >= 2)
                        openAccessManagement();
                    else if (choice == 0)
                    {
                        const auto point = *_accessPoint;
                        if (Handheld::RideManagement::Resolve(point.target)->status != RideStatus::closed)
                        {
                            _status = "Close ride before moving access";
                            invalidate();
                            break;
                        }
                        launchRideTool([=] { return Handheld::RideAccess::BeginRelocation(point); });
                        if (Handheld::FlatRides::CurrentStage() == Handheld::FlatRides::Stage::inactive)
                            GetWindowManager()->ShowError("Cannot move access", "Access changed; select it again");
                        else
                            Handheld::RideManagement::Locate(point.target);
                    }
                    else
                    {
                        if (_pausedHere && (gGamePaused & GAME_PAUSED_NORMAL))
                        {
                            PauseToggle();
                            _ridesReviewResumed = true;
                        }
                        std::string error;
                        auto draft = Handheld::RideAccess::Review(*_accessPoint, {}, error);
                        if (draft)
                            openAccessReview(*draft);
                        else
                        {
                            _status = error;
                            invalidate();
                        }
                    }
                    break;
                case Page::accessReview:
                    if (choice != 0)
                    {
                        cancelAccessReview();
                        break;
                    }
                    if (_accessDraft)
                    {
                        if (_pausedHere && (gGamePaused & GAME_PAUSED_NORMAL))
                        {
                            PauseToggle();
                            _ridesReviewResumed = true;
                        }
                        auto action = _accessDraft->action;
                        const auto request = ++_ridesRequest;
                        const auto target = _accessDraft->point.target;
                        _ridesBusy = true;
                        action.SetCallback([request, target, lifetime = std::weak_ptr<int>(_ridesLifetime)](
                                               const auto*, const GameActions::Result* result) {
                            const auto alive = lifetime.lock();
                            if (!alive)
                                return;
                            auto* base = GetWindowManager()->FindByClass(WindowClass::handheldMenu);
                            if (!base)
                                return;
                            auto* window = static_cast<HandheldMenuWindow*>(base);
                            if (window->_ridesLifetime != alive || window->_ridesRequest != request
                                || window->_page != Page::accessReview)
                                return;
                            window->_ridesBusy = false;
                            if (result->error == GameActions::Status::ok)
                            {
                                Handheld::EndTerrain();
                                window->_accessDraft.reset();
                                window->openRideAccess(target);
                                window->_status = "Access edited; check queue / path connections";
                            }
                            else
                                window->_status = result->getErrorMessage().empty() ? result->getErrorTitle()
                                                                                    : result->getErrorMessage();
                            window->invalidate();
                        });
                        const auto result = GameActions::Execute(&action, getGameState());
                        if (result.error != GameActions::Status::ok)
                        {
                            _ridesBusy = false;
                            _status = result.getErrorMessage().empty() ? result.getErrorTitle() : result.getErrorMessage();
                        }
                        invalidate();
                    }
                    break;
                case Page::ridesAccess:
                    if (_ridesTarget && Handheld::FlatRides::CanResume(*_ridesTarget))
                    {
                        if (choice == 0)
                        {
                            const auto target = *_ridesTarget;
                            launchRideTool([=] { return Handheld::ResumeFlatRide(target.id); });
                            Handheld::RideManagement::Locate(target);
                        }
                        else if (choice == 1)
                            page(Page::ridesInspect);
                        else
                            close();
                        break;
                    }
                    if (choice == 0)
                        close();
                    else if (!_ridesTarget || !Handheld::RideAccess::Inspect(*_ridesTarget))
                    {
                        _status = "Ride/access changed; return to inspection";
                        invalidate();
                    }
                    else if (choice == 1 || choice == 2)
                    {
                        const auto target = *_ridesTarget;
                        const auto exit = choice == 2;
                        launchRideTool([=] { return Handheld::RideAccess::BeginPaths(target, exit); });
                    }
                    else if (choice == 5 || choice == 6)
                    {
                        const auto target = *_ridesTarget;
                        const auto exit = choice == 6;
                        launchRideTool([=] { return Handheld::RideAccess::BeginPaths(target, exit, true); });
                    }
                    else if (choice == 3)
                    {
                        _ridesDraft = Handheld::RideManagement::Begin(*_ridesTarget, RideStatus::open);
                        if (_ridesDraft)
                            page(Page::ridesConfirm);
                    }
                    else if (choice == 4)
                        page(Page::ridesInspect);
                    break;
                case Page::ridesVehicles:
                case Page::ridesAppearance:
                case Page::ridesMusic:
                case Page::ridesDispatch:
                case Page::ridesOperations:
                case Page::ridesMaintenance:
                {
                    const auto* ride = _ridesTarget ? Handheld::RideManagement::Resolve(*_ridesTarget) : nullptr;
                    const auto settings = ride
                        ? (_page == Page::ridesAppearance     ? Handheld::RideManagement::AppearanceSettings(*ride)
                               : _page == Page::ridesMusic    ? Handheld::RideManagement::MusicSettings(*ride)
                               : _page == Page::ridesVehicles ? Handheld::RideManagement::VehicleSettings(*ride)
                               : _page == Page::ridesDispatch
                               ? Handheld::RideManagement::DispatchSettings(*ride)
                               : Handheld::RideManagement::Settings(*ride, _page == Page::ridesMaintenance))
                        : std::vector<Handheld::RideManagement::Setting>{};
                    const auto appearanceRows = _page == Page::ridesAppearance ? ridesPresentationFor(_page, choice).rows
                                                                               : std::vector<std::string>{};
                    if (_page == Page::ridesAppearance && choice < appearanceRows.size()
                        && appearanceRows[choice] == "Randomize vehicle colours...")
                    {
                        _ridesDraft = Handheld::RideManagement::BeginRandomColours(*_ridesTarget);
                        if (_ridesDraft)
                            page(Page::ridesConfirm);
                    }
                    else if (
                        ride && _page == Page::ridesAppearance && choice < appearanceRows.size()
                        && appearanceRows[choice] == "Paint track scheme...")
                    {
                        const auto target = *_ridesTarget;
                        close();
                        Handheld::BeginTrackColour(target);
                    }
                    else if (
                        ride && _page == Page::ridesMaintenance && !ride->getRideTypeDescriptor().availableBreakdowns.isEmpty()
                        && choice == settings.size())
                    {
                        if (Handheld::RideManagement::LocateMechanic(*_ridesTarget))
                            close();
                        else
                        {
                            _status = "Unable to locate mechanic";
                            invalidate();
                        }
                    }
                    else if (
                        ride && _page == Page::ridesMaintenance && !ride->getRideTypeDescriptor().availableBreakdowns.isEmpty()
                        && choice == settings.size() + 1)
                    {
                        _ridesDraft = Handheld::RideManagement::BeginRefurbishment(*_ridesTarget);
                        if (_ridesDraft)
                        {
                            if (_pausedHere && (gGamePaused & GAME_PAUSED_NORMAL))
                            {
                                PauseToggle();
                                _ridesReviewResumed = true;
                            }
                            page(Page::ridesConfirm);
                        }
                    }
                    else if (choice >= settings.size())
                        page(ride ? Page::ridesInspect : Page::ridesRoster);
                    else
                    {
                        _ridesDraft = Handheld::RideManagement::BeginSetting(*_ridesTarget, settings[choice]);
                        if (_ridesDraft)
                            page(Page::ridesPrice);
                    }
                    break;
                }
                case Page::ridesGraphs:
                    if (choice == 4)
                        page(Page::ridesStats);
                    else
                    {
                        _rideGraphMetric = choice;
                        invalidate();
                    }
                    break;
                case Page::ridesStations:
                    page(Page::ridesCustomers);
                    break;
                case Page::ridesCustomers:
                    if (choice >= 2 && choice <= 4 && _ridesTarget && Handheld::RideManagement::Resolve(*_ridesTarget))
                    {
                        _guestFilter = {};
                        _guestFilter.ride = Handheld::GuestManagement::RideFilter{
                            *_ridesTarget, static_cast<Handheld::GuestManagement::RideFilterKind>(choice - 2)
                        };
                        _guestRosterParent = Page::ridesCustomers;
                        _guestRosterRideParent = _ridesTarget;
                        _guestRosterFocus = 3;
                        _dashboard.section = Handheld::Dashboard::Section::guests;
                        page(Page::guestRoster);
                    }
                    else
                        page(choice == 0 ? Page::ridesStations : Page::ridesStats);
                    break;
                case Page::ridesEarnings:
                    page(Page::ridesStats);
                    break;
                case Page::ridesStats:
                    if (choice == 2 && _ridesTarget)
                    {
                        Handheld::RideManagement::BeginGraphs(*_ridesTarget);
                        _rideGraphOffset = _rideGraphMetric = 0;
                    }
                    page(
                        choice == 0                                                            ? Page::ridesCustomers
                            : choice == 1                                                      ? Page::ridesEarnings
                            : choice == 2                                                      ? Page::ridesGraphs
                            : _ridesTarget && Handheld::RideManagement::Resolve(*_ridesTarget) ? Page::ridesInspect
                                                                                               : Page::ridesRoster);
                    break;
                case Page::designScenery:
                    if (choice == 0)
                        page(Page::designName);
                    else
                    {
                        const auto result = choice < 3 ? Handheld::DesignAuthoring::SetNearby(choice == 1)
                                                       : Handheld::DesignAuthoring::ToggleScenery(choice - 3);
                        _status = result.error == GameActions::Status::ok ? "Design scenery updated" : result.getErrorMessage();
                        invalidate();
                    }
                    break;
                case Page::designName:
                    if (choice == 47)
                        page(Page::designScenery);
                    else if (choice == 46)
                    {
                        const auto result = Handheld::DesignAuthoring::Review(_nameEntry.text);
                        if (result.error == GameActions::Status::ok)
                            page(Page::designExport);
                        else
                            _status = result.getErrorMessage();
                        invalidate();
                    }
                    else
                    {
                        _nameEntry.Press(choice);
                        invalidate();
                    }
                    break;
                case Page::designExport:
                    if (choice == 0)
                    {
                        Handheld::DesignAuthoring::CancelReview();
                        page(Page::designName);
                    }
                    else
                    {
                        const auto filename = Handheld::DesignAuthoring::Filename();
                        const auto result = Handheld::DesignAuthoring::Confirm();
                        page(Page::ridesInspect);
                        _status = result.error == GameActions::Status::ok ? "Saved " + filename : result.getErrorMessage();
                        invalidate();
                    }
                    break;
                case Page::ridesName:
                    if (choice == 47)
                        page(_ridesTarget && _ridesTarget->shop ? Page::shop : Page::ridesInspect);
                    else if (choice == 46 && _ridesDraft)
                    {
                        _ridesDraft->name = _nameEntry.text;
                        applyRideDraft();
                    }
                    else
                    {
                        _nameEntry.Press(choice);
                        invalidate();
                    }
                    break;
                case Page::ridesPrice:
                    if (!_ridesDraft)
                    {
                        page(Page::ridesRoster);
                        break;
                    }
                    if (rideVehicleChoice())
                    {
                        if (choice < _ridesDraft->setting->values.size())
                        {
                            _ridesDraft->value = _ridesDraft->setting->values[choice];
                            page(Page::ridesConfirm);
                        }
                    }
                    else if (_ridesDraft->setting && _ridesDraft->setting->appearance)
                    {
                        if (choice < _ridesDraft->setting->values.size())
                        {
                            _ridesDraft->value = _ridesDraft->setting->values[choice];
                            applyRideDraft();
                        }
                    }
                    else if (choice == 0)
                    {
                        if (_ridesDraft->setting && _ridesDraft->setting->vehicle.has_value())
                            page(Page::ridesConfirm);
                        else
                            applyRideDraft();
                    }
                    else
                        page(ridesDraftParent());
                    break;
                case Page::ridesConfirm:
                    if (choice == 1)
                        page(ridesDraftParent());
                    else
                        applyRideDraft();
                    break;
                case Page::guests:
                    // Rows: roster, thoughts, actions, tracked, (hidden) back.
                    if (choice == 1 || choice == 2)
                    {
                        _guestGroupMode = choice == 2 ? Handheld::GuestManagement::GroupMode::actions
                                                      : Handheld::GuestManagement::GroupMode::thoughts;
                        _guestGroupFocus = 0;
                        page(Page::guestGroups);
                    }
                    else if (choice == 0 || choice == 3)
                    {
                        _guestFilter = { {}, choice == 3 };
                        _guestRosterParent = Page::guests;
                        _guestRosterFocus = 3;
                        page(Page::guestRoster);
                    }
                    else
                        page(Page::dashboard);
                    break;
                case Page::guestGroups:
                    _guestGroupFocus = choice;
                    if (choice >= _guestGroups.size())
                        page(Page::guests);
                    else
                    {
                        _guestFilter = { _guestGroups[choice], false };
                        _guestRosterParent = Page::guestGroups;
                        _guestRosterFocus = 3;
                        page(Page::guestRoster);
                    }
                    break;
                case Page::guestRoster:
                    _guestRosterFocus = _focus.index();
                    if (choice == _guestRoster.size())
                    {
                        _parkMap.Begin();
                        _parkMap.mode = 2;
                        _parkMap.rides = false;
                        _parkMap.filter = _guestFilter.tracked ? 1 : 0;
                        _parkMap.guestFilter = _guestFilter;
                        _parkMap.guestFilter->tracked = false;
                        _guestMapReturn = true;
                        page(Page::parkMap);
                    }
                    else if (choice == _guestRoster.size() + 1)
                    {
                        _nameEntry = { _guestFilter.name };
                        page(Page::guestFilterName);
                    }
                    else if (choice == _guestRoster.size() + 2)
                    {
                        _guestFilter.tracked = !_guestFilter.tracked;
                        page(Page::guestRoster);
                    }
                    else if (choice > _guestRoster.size())
                    {
                        if (_guestRosterParent == Page::ridesCustomers)
                        {
                            _dashboard.section = Handheld::Dashboard::Section::rides;
                            _ridesTarget = _guestRosterRideParent;
                        }
                        page(_guestRosterParent);
                    }
                    else if (Handheld::GuestManagement::Resolve(_guestRoster[choice]))
                    {
                        _guestTarget = _guestRoster[choice];
                        _guestTabs = {};
                        _guestFromPark = false;
                        page(Page::guestInspect);
                    }
                    else
                    {
                        _status = "Guest no longer in park";
                        invalidate();
                    }
                    break;
                case Page::guestRideHistory:
                    if (choice >= _guestRides.size())
                        back();
                    else if (_guestTarget && Handheld::GuestManagement::HistoryRideCurrent(*_guestTarget, _guestRides[choice]))
                    {
                        _guestRideFocus = choice;
                        _ridesTarget = _guestRides[choice];
                        _guestRideInspection = true;
                        _dashboard.section = Handheld::Dashboard::Section::rides;
                        _dashboard.focus = Handheld::Dashboard::Focus::content;
                        page(Page::ridesInspect);
                    }
                    else
                    {
                        _status = "Ride history changed; choose again";
                        invalidate();
                    }
                    break;
                case Page::guestInspect:
                    if (!_guestTarget || !Handheld::GuestManagement::Resolve(*_guestTarget))
                    {
                        page(Page::guestRoster);
                        break;
                    }
                    if (choice == 0)
                    {
                        const auto target = *_guestTarget;
                        if (WindowGetMain() && Handheld::GuestManagement::Location(target))
                        {
                            close();
                            Handheld::GuestManagement::Locate(target);
                            return;
                        }
                        _status = "Location unavailable";
                        invalidate();
                    }
                    else if (choice == 6)
                    {
                        _guestNameBaseline = Handheld::GuestManagement::Resolve(*_guestTarget)->getName();
                        _nameEntry = { _guestNameBaseline };
                        page(Page::guestName);
                    }
                    else if (choice == 7)
                    {
                        _guestFlagsBaseline = Handheld::GuestManagement::Resolve(*_guestTarget)->peepFlags;
                        _guestTrackingValue = !_guestFlagsBaseline.has(PeepFlag::tracking);
                        const auto result = Handheld::GuestManagement::SetTracking(
                            *_guestTarget, _guestFlagsBaseline, _guestTrackingValue, true);
                        _status = result.error == GameActions::Status::ok ? "Tracking updated"
                                                                          : Handheld::Finances::Feedback(result);
                        invalidate();
                    }
                    else if (choice == 9)
                    {
                        const auto person = *_guestTarget;
                        if (!Handheld::GuestManagement::Resolve(person)->canBePickedUp())
                        {
                            _status = "Person cannot be picked up now";
                            break;
                        }
                        saveGuestReturn();
                        close();
                        if (!Handheld::BeginPeepPlacement({ person, {} }))
                            HandheldGuestOpen(person);
                        return;
                    }
                    else if (choice == 11)
                    {
                        if (Handheld::GuestFollow::Current() == _guestTarget)
                        {
                            Handheld::GuestFollow::Stop();
                            _status = "Follow stopped";
                            invalidate();
                        }
                        else if (Handheld::GuestFollow::Start(*_guestTarget))
                        {
                            saveGuestReturn();
                            close();
                            return;
                        }
                        else
                        {
                            _status = "Guest cannot be followed now";
                            invalidate();
                        }
                    }
                    else if (choice == 10)
                    {
                        _guestRideFocus = 0;
                        page(Page::guestRideHistory);
                    }
                    else if (choice == 8)
                        page(Page::guestPreferences);
                    else
                        page(
                            choice == 1       ? Page::guestNeeds
                                : choice == 2 ? Page::guestThoughts
                                : choice == 3 ? Page::guestSpending
                                : choice == 4 ? Page::guestInventory
                                              : Page::guestRoster);
                    break;
                case Page::guestFilterName:
                    if (choice == 47)
                        page(Page::guestRoster);
                    else if (choice == 46)
                    {
                        _guestFilter.name = _nameEntry.text;
                        _guestRosterFocus = 3;
                        page(Page::guestRoster);
                    }
                    else
                    {
                        _nameEntry.Press(choice);
                        invalidate();
                    }
                    break;
                case Page::guestName:
                    if (choice == 47)
                        page(Page::guestInspect);
                    else if (choice == 46 && _guestTarget)
                    {
                        const auto result = Handheld::GuestManagement::SetName(
                            *_guestTarget, _guestNameBaseline, _nameEntry.text, true);
                        if (result.error == GameActions::Status::ok)
                            page(Page::guestInspect);
                        else
                            _status = Handheld::Finances::Feedback(result);
                    }
                    else
                        _nameEntry.Press(choice);
                    break;
                case Page::guestNameConfirm:
                case Page::guestTracking:
                    if (choice == 1)
                        page(_page == Page::guestTracking ? Page::guestInspect : Page::guestName);
                    else if (_guestTarget)
                    {
                        const auto result = _page == Page::guestTracking
                            ? Handheld::GuestManagement::SetTracking(
                                  *_guestTarget, _guestFlagsBaseline, _guestTrackingValue, true)
                            : Handheld::GuestManagement::SetName(*_guestTarget, _guestNameBaseline, _nameEntry.text, true);
                        if (result.error == GameActions::Status::ok)
                            page(Page::guestInspect);
                        else
                            _status = Handheld::Finances::Feedback(result);
                    }
                    break;
                case Page::guestPreferences:
                case Page::guestNeeds:
                case Page::guestSpending:
                    page(
                        _guestTarget && Handheld::GuestManagement::Resolve(*_guestTarget) ? Page::guestInspect
                                                                                          : Page::guestRoster);
                    break;
                case Page::guestThoughts:
                case Page::guestInventory:
                    if (_guestTarget)
                    {
                        const auto* guest = Handheld::GuestManagement::Resolve(*_guestTarget);
                        if (!guest)
                        {
                            page(Page::guestRoster);
                            break;
                        }
                        const auto values = _page == Page::guestThoughts ? Handheld::GuestManagement::Thoughts(*guest)
                                                                         : Handheld::GuestManagement::Inventory(*guest);
                        if (choice >= values.size())
                            page(Page::guestInspect);
                        else
                        {
                            _guestText = values[choice];
                            _guestTextParent = _page;
                            _guestTextPage = 0;
                            page(Page::guestText);
                        }
                    }
                    else
                        page(Page::guestRoster);
                    break;
                case Page::guestText:
                    if (!_guestTarget || !Handheld::GuestManagement::Resolve(*_guestTarget))
                    {
                        page(Page::guestRoster);
                        break;
                    }
                    if (rows().size() == 1 || choice == 2)
                        page(_guestTextParent);
                    else
                    {
                        const auto pages = std::max<size_t>(
                            1, (Handheld::GuestManagement::TextLines(_guestText).size() + 5) / 6);
                        if (choice == 0 && _guestTextPage)
                            --_guestTextPage;
                        if (choice == 1 && _guestTextPage + 1 < pages)
                            ++_guestTextPage;
                        invalidate();
                    }
                    break;
                case Page::marketing:
                    if (_marketingTabs.tab == 0)
                    {
                        _marketingActiveTypes = Handheld::MarketingManagement::ActiveTypes();
                        if (_marketingActiveTypes.empty())
                        {
                            marketingTab(1);
                            break;
                        }
                        if (choice < _marketingActiveTypes.size())
                        {
                            _marketingActiveType = _marketingActiveTypes[choice];
                            page(Page::marketingActive);
                        }
                    }
                    else if (choice < ADVERTISING_CAMPAIGN_COUNT && Handheld::MarketingManagement::Eligible(choice))
                    {
                        _marketingDraft = Handheld::MarketingManagement::Draft{ static_cast<int>(choice), 2, {} };
                        const auto targets = Handheld::MarketingManagement::Targets(choice);
                        if (targets.size() == 1)
                            Handheld::MarketingManagement::SelectTarget(*_marketingDraft, targets.front());
                        page(Page::marketingEditor);
                    }
                    else
                    {
                        _status = "Campaign unavailable; check its eligibility label";
                        invalidate();
                    }
                    break;
                case Page::marketingActive:
                    page(Page::marketing);
                    break;
                case Page::marketingTypes:
                    if (choice >= _marketingTypes.size())
                        page(Page::marketing);
                    else if (Handheld::MarketingManagement::Eligible(_marketingTypes[choice]))
                    {
                        _marketingDraft = Handheld::MarketingManagement::Draft{ _marketingTypes[choice], 2, {} };
                        page(Page::marketingEditor);
                    }
                    else
                    {
                        _status = "Campaign no longer available";
                        invalidate();
                    }
                    break;
                case Page::marketingEditor:
                    if (_marketingDraft)
                    {
                        const size_t duration = Handheld::MarketingManagement::NeedsTarget(_marketingDraft->type) ? 1 : 0;
                        if (duration && choice == 0)
                            page(Page::marketingTargets);
                        else if (choice == duration)
                        {
                            Handheld::MarketingManagement::AdjustWeeks(
                                *_marketingDraft, _marketingDraft->weeks == 12 ? -10 : 1);
                            _status.clear();
                            invalidate();
                        }
                        else if (choice == duration + 2)
                            page(Page::marketing);
                        else if (choice == duration + 1)
                        {
                            const auto result = Handheld::MarketingManagement::Action(*_marketingDraft, false);
                            if (result.error == GameActions::Status::ok)
                                page(Page::marketingConfirm);
                            else
                            {
                                _status = Handheld::Finances::Feedback(result);
                                invalidate();
                            }
                        }
                    }
                    break;
                case Page::marketingTargets:
                    if (choice >= _marketingTargets.size())
                        page(Page::marketingEditor);
                    else if (
                        _marketingDraft
                        && Handheld::MarketingManagement::SelectTarget(*_marketingDraft, _marketingTargets[choice]))
                        page(Page::marketingEditor);
                    else
                    {
                        _status = "Target changed; choose again";
                        invalidate();
                    }
                    break;
                case Page::marketingConfirm:
                    if (choice == 1)
                        page(Page::marketing);
                    else if (_marketingDraft)
                    {
                        _marketingActiveType = _marketingDraft->type;
                        _marketingBusy = true;
                        const auto result = Handheld::MarketingManagement::Action(
                            *_marketingDraft, true,
                            [this, lifetime = std::weak_ptr<int>(_marketingLifetime)](
                                const GameActions::GameAction*, const GameActions::Result* outcome) {
                                if (lifetime.expired())
                                    return;
                                _marketingBusy = false;
                                if (_page == Page::marketing)
                                {
                                    page(Page::marketing);
                                    const auto active = Handheld::MarketingManagement::ActiveTypes();
                                    const auto found = std::find(active.begin(), active.end(), _marketingActiveType);
                                    if (found != active.end())
                                        _focus.move(static_cast<int>(found - active.begin()), rows().size());
                                    _status = outcome->error == GameActions::Status::ok
                                        ? "Campaign started"
                                        : Handheld::Finances::Feedback(*outcome);
                                    invalidate();
                                }
                            });
                        if (result.error == GameActions::Status::ok)
                        {
                            _marketingTabs.tab = 0;
                            page(Page::marketing);
                            const auto active = Handheld::MarketingManagement::ActiveTypes();
                            const auto found = std::find(active.begin(), active.end(), _marketingActiveType);
                            if (found != active.end())
                                _focus.move(static_cast<int>(found - active.begin()), rows().size());
                            _status = _marketingBusy ? "Campaign start requested" : "Campaign started";
                        }
                        else
                        {
                            _marketingBusy = false;
                            _status = Handheld::Finances::Feedback(result);
                        }
                        invalidate();
                    }
                    break;
                case Page::researchCatalog:
                    if (choice < _researchCatalog.size())
                    {
                        _researchItem = _researchCatalog[choice];
                        _researchCatalogFocus = choice;
                        page(Page::researchItem);
                    }
                    else if (choice == _researchCatalog.size())
                    {
                        _researchFilter = (_researchFilter + 1) % 8;
                        _researchCatalog = Handheld::ResearchCatalog::Roster(_researchFilter);
                        _focus = {};
                        invalidate();
                    }
                    else
                        page(Page::research);
                    break;
                case Page::researchItem:
                    if (choice == 0 && _researchItem
                        && Handheld::ResearchCatalog::Construction(*_researchItem)
                            != Handheld::ResearchCatalog::BuildKind::none)
                    {
                        const auto target = *_researchItem;
                        bool started = false;
                        switch (Handheld::ResearchCatalog::Construction(target))
                        {
                            case Handheld::ResearchCatalog::BuildKind::shop:
                                started = Handheld::SetShopSelection(target.item.entryIndex);
                                break;
                            case Handheld::ResearchCatalog::BuildKind::flat:
                                started = Handheld::BeginFlatRide(target.item.entryIndex);
                                break;
                            case Handheld::ResearchCatalog::BuildKind::maze:
                                started = Handheld::BeginMaze(target.item.entryIndex);
                                break;
                            case Handheld::ResearchCatalog::BuildKind::track:
                                started = Handheld::BeginCustomTrack(target.item.entryIndex, target.item.baseRideType);
                                break;
                            default:
                                break;
                        }
                        if (started)
                            close();
                        else
                        {
                            _status = "Construction unavailable; select again";
                            invalidate();
                        }
                    }
                    else
                        back();
                    break;
                case Page::research:
                    if (_researchTabs.tab == 0)
                    {
                        // Rows: funding switch, priorities, catalog.
                        if (choice == 0)
                            stepFunding(1, true);
                        else if (choice == 1)
                            researchTab(1);
                        else
                        {
                            _researchFilter = 0;
                            _researchCatalog = Handheld::ResearchCatalog::Roster(0);
                            page(Page::researchCatalog);
                        }
                    }
                    else if (_researchDraft)
                    {
                        if (choice < 7)
                        {
                            Handheld::ResearchManagement::ToggleCategory(*_researchDraft, choice);
                            _status.clear();
                            invalidate();
                        }
                        else if (choice == 8)
                        {
                            _researchDraft = Handheld::ResearchManagement::Begin();
                            _status = "Draft discarded";
                            invalidate();
                        }
                        else
                        {
                            const bool changed = _researchDraft->funding != _researchDraft->baselineFunding
                                || _researchDraft->priorities != _researchDraft->baselinePriorities;
                            const auto query = Handheld::ResearchManagement::Action(*_researchDraft, false);
                            if (changed && query.error == GameActions::Status::ok)
                                page(Page::researchConfirm);
                            else
                            {
                                _status = changed ? Handheld::Finances::Feedback(query) : "No changes pending";
                                invalidate();
                            }
                        }
                    }
                    break;
                case Page::researchEditor:
                    if (choice == 9)
                        page(Page::research);
                    else if (_researchDraft)
                    {
                        if (choice == 0)
                        {
                            Handheld::ResearchManagement::AdjustFunding(*_researchDraft, _researchDraft->funding == 3 ? -3 : 1);
                            _status.clear();
                        }
                        else if (choice < 8)
                        {
                            Handheld::ResearchManagement::ToggleCategory(*_researchDraft, choice - 1);
                            _status.clear();
                        }
                        else
                        {
                            const auto result = Handheld::ResearchManagement::Action(*_researchDraft, false);
                            if (result.error == GameActions::Status::ok)
                                page(Page::researchConfirm);
                            else
                                _status = Handheld::Finances::Feedback(result);
                        }
                        invalidate();
                    }
                    break;
                case Page::researchConfirm:
                    if (choice == 1)
                    {
                        page(Page::research);
                    }
                    else if (choice == 0 && _researchDraft)
                    {
                        _civicBusy = true;
                        const auto result = Handheld::ResearchManagement::Action(
                            *_researchDraft, true,
                            [this,
                             lifetime = std::weak_ptr<int>(_civicLifetime)](const auto*, const GameActions::Result* outcome) {
                                if (lifetime.expired())
                                    return;
                                _civicBusy = false;
                                if (_page != Page::researchConfirm)
                                    return;
                                if (outcome->error == GameActions::Status::ok)
                                {
                                    _researchDraft = Handheld::ResearchManagement::Begin();
                                    page(Page::research);
                                    _status = "Research plan applied";
                                }
                                else
                                    _status = Handheld::Finances::Feedback(*outcome);
                                invalidate();
                            });
                        if (result.error != GameActions::Status::ok)
                        {
                            _civicBusy = false;
                            _status = Handheld::Finances::Feedback(result);
                        }
                        else if (_civicBusy)
                            _status = "Applying research plan...";
                        invalidate();
                    }
                    break;
                case Page::shop:
                {
                    const size_t priceRows = shopPriceCount();
                    if (choice >= 1 + priceRows)
                        choice = choice - priceRows + 1; // Map the grid onto 2..4.
                    else if (choice >= 1)
                    {
                        invalidate();
                        break;
                    } // Prices change with Left/Right.
                    if (!_shop || !Handheld::ShopManagement::Find(*_shop))
                        _status = "Shop no longer available";
                    else if (choice == 4)
                    {
                        page(Page::shopAppearance);
                        _shopAppearanceChannel = 0;
                    }
                    else if (choice == 3)
                    {
                        _ridesTarget = Handheld::RideManagement::Identify(_shop->id, true);
                        if (_ridesTarget)
                            _ridesDraft = Handheld::RideManagement::BeginName(*_ridesTarget);
                        if (_ridesDraft)
                        {
                            _nameEntry = { _ridesDraft->name };
                            page(Page::ridesName);
                        }
                    }
                    else if (choice == 2)
                        page(Page::shopDemolish);
                    else
                    {
                        const auto result = Handheld::ShopManagement::SetStatus(
                            *_shop, Handheld::ShopManagement::Find(*_shop)->status != RideStatus::open);
                        _status = result.error == GameActions::Status::ok ? "" : result.getErrorMessage();
                    }
                    invalidate();
                    break;
                }
                case Page::shopAppearance:
                    if (!_shop || choice >= Handheld::ShopAppearance::Settings(*_shop).size())
                        page(Page::shop);
                    else
                    {
                        _shopAppearanceChannel = choice;
                        _shopAppearanceDraft = Handheld::ShopAppearance::Begin(*_shop, choice);
                        if (_shopAppearanceDraft)
                        {
                            page(Page::shopAppearanceEditor);
                            const auto& d = *_shopAppearanceDraft;
                            const auto it = std::find(d.setting.values.begin(), d.setting.values.end(), d.value);
                            if (it != d.setting.values.end())
                                _focus.move(static_cast<int>(it - d.setting.values.begin()), rows().size());
                        }
                    }
                    break;
                case Page::shopAppearanceEditor:
                    if (_shopAppearanceDraft && choice < _shopAppearanceDraft->setting.values.size())
                    {
                        _shopAppearanceDraft->value = _shopAppearanceDraft->setting.values[choice];
                        const auto q = Handheld::ShopAppearance::Action(*_shopAppearanceDraft, false);
                        if (q.error == GameActions::Status::ok)
                            applyShopAppearanceDraft();
                        else
                        {
                            _status = q.getErrorMessage();
                            invalidate();
                        }
                    }
                    break;
                case Page::shopAppearanceReview:
                    if (choice == 0)
                    {
                        back();
                        break;
                    }
                    applyShopAppearanceDraft();
                    break;
                case Page::shopDemolish:
                    if (choice == 1)
                        page(Page::shop);
                    else if (_shop)
                    {
                        const auto result = Handheld::ShopManagement::Demolish(*_shop, true);
                        if (result.error == GameActions::Status::ok)
                            close();
                        else
                        {
                            _status = result.getErrorMessage();
                            invalidate();
                        }
                    }
                    break;
                case Page::ride:
                    if (_ridesTarget && Handheld::RideAccess::Supported(*_ridesTarget))
                    {
                        const size_t index = Handheld::FlatRides::CanResume(*_ridesTarget) ? 1 : 0;
                        if (choice == index)
                        {
                            openAccessManagement();
                            break;
                        }
                        if (choice > index)
                            --choice;
                    }
                    if (_ridesTarget && Handheld::FlatRides::CanResume(*_ridesTarget))
                    {
                        if (choice == 0)
                        {
                            const auto target = *_ridesTarget;
                            launchRideTool([=] { return Handheld::ResumeFlatRide(target.id); });
                            Handheld::RideManagement::Locate(target);
                            break;
                        }
                        --choice;
                    }
                    if (_ridesTarget && _focus.index() < rows().size() && rows()[_focus.index()] == "Edit maze...")
                    {
                        const auto t = *_ridesTarget;
                        launchRideTool([=] { return Handheld::ResumeMaze(t.id); });
                    }
                    else if (_ridesTarget && _focus.index() < rows().size() && rows()[_focus.index()] == "Edit track...")
                    {
                        if (_pausedHere && (gGamePaused & GAME_PAUSED_NORMAL))
                        {
                            PauseToggle();
                            _pausedHere = false;
                        }
                        _trackBlocks = Handheld::TrackEditing::Blocks(*_ridesTarget);
                        _trackBlock.reset();
                        _dashboard.section = Handheld::Dashboard::Section::rides;
                        _dashboard.focus = Handheld::Dashboard::Focus::content;
                        page(Page::trackBlocks);
                    }
                    else if (
                        _ridesTarget && Handheld::CustomTracks::CanResume(*_ridesTarget)
                        && choice == (Handheld::RideAccess::Supported(*_ridesTarget) ? 4u : 3u))
                    {
                        const auto target = *_ridesTarget;
                        launchRideTool([=] { return Handheld::ResumeCustomTrack(target.id); });
                    }
                    else if (choice == 3 && _ridesTarget && Handheld::RideAccess::Supported(*_ridesTarget))
                        openRideAccess(*_ridesTarget);
                    else if (choice == 2)
                        close();
                    else
                    {
                        auto action = GameActions::RideSetStatusAction(
                            _rideId, choice == 0 ? RideStatus::open : RideStatus::closed);
                        const auto result = GameActions::Execute(&action, getGameState());
                        _status = result.error == GameActions::Status::ok ? "Ride status updated." : result.getErrorMessage();
                        invalidate();
                    }
                    break;
                case Page::saveName:
                    if (choice == 47)
                    {
                        _saveDraft.reset();
                        page(Page::saveFiles);
                    }
                    else if (choice == 46)
                    {
                        std::string error;
                        _saveDraft = Handheld::NamedSave::Draft::Review(_files.directory, _nameEntry.text, error);
                        if (_saveDraft)
                            page(Page::saveReview);
                        else
                        {
                            _status = error;
                            invalidate();
                        }
                    }
                    else
                    {
                        _nameEntry.Press(choice);
                        invalidate();
                    }
                    break;
                case Page::saveReview:
                    if (choice == 0)
                        save();
                    else
                    {
                        _saveDraft.reset();
                        page(Page::saveName);
                    }
                    break;
                case Page::saveFiles:
                    if (choice == 0)
                        editSaveName(fs::u8path(gScenarioSavePath).stem().u8string());
                    else if (choice == 1)
                    {
                        _files.Parent();
                        _fileFocus = 0;
                        page(Page::saveFiles);
                    }
                    else if (choice - 1 - Handheld::FileBrowsing::kFirst < _files.entries.size())
                    {
                        const auto entry = _files.entries[choice - 1 - Handheld::FileBrowsing::kFirst];
                        if (entry.directory)
                        {
                            _files.Open(entry.path, _files.pattern);
                            _fileFocus = 0;
                            page(Page::saveFiles);
                        }
                        else
                            editSaveName(entry.path.stem().u8string());
                    }
                    break;
                case Page::save:
                {
                    if (choice == 3)
                    {
                        openSaveFiles();
                        break;
                    }
                    _namedSave = false;
                    _saveDraft.reset();
                    _slot = choice;
                    std::error_code error;
                    if (fs::exists(SlotPath(_slot), error))
                        page(Page::overwrite);
                    else
                        save();
                    break;
                }
                case Page::parkInspect:
                    if (choice == 1)
                        page(Page::parkFiles);
                    else if (!_loadFile || !Handheld::FileBrowsing::Current(*_loadFile))
                    {
                        _status = "File changed. Back and select again.";
                        invalidate();
                    }
                    else if (gLegacyScene == LegacyScene::playing)
                        page(Page::discard);
                    else
                        load();
                    break;
                case Page::designFiles:
                    if (choice == 0)
                    {
                        const auto child = _files.directory;
                        _files.Parent();
                        _fileFocus = 0;
                        for (size_t i = 0; i < _files.entries.size(); ++i)
                            if (_files.entries[i].path == child)
                                _fileFocus = i + Handheld::FileBrowsing::kFirst;
                        page(Page::designFiles);
                    }
                    else if (choice - Handheld::FileBrowsing::kFirst < _files.entries.size())
                    {
                        const auto entry = _files.entries[choice - Handheld::FileBrowsing::kFirst];
                        if (entry.directory)
                        {
                            _files.Open(entry.path, _files.pattern);
                            _fileFocus = 0;
                            page(Page::designFiles);
                        }
                        else
                        {
                            _designFile = Handheld::FileBrowsing::Capture(entry.path);
                            _designChoice.reset();
                            _designDetail = 0;
                            _designMetadata.reset();
                            _designFilePreview.reset();
                            _designFileStatus.clear();
                            if (_designFile)
                                _designMetadata = TrackDesignImport(entry.path.u8string().c_str());
                            if (!_designFile)
                                _designFileStatus = "File removed or unreadable";
                            else if (!_designMetadata)
                                _designFileStatus = "This design file could not be read";
                            else if (!Handheld::Designs::Eligible(*_designMetadata))
                                _designFileStatus = Handheld::Designs::UnavailableReason(*_designMetadata);
                            else
                                _designFilePreview = Handheld::Designs::FileEntry(entry.path.u8string(), {}, 260, 144);
                            page(Page::designInspect);
                        }
                    }
                    break;
                case Page::designWarnings:
                    if (choice + 1 == rows().size())
                        page(_designDetailsFromFile ? Page::designInspect : Page::designOptions);
                    else if (choice + 2 == rows().size())
                    {
                        _designTechnical = !_designTechnical;
                        _designDetail = 0;
                        const auto count = rows().size();
                        _focus.move(static_cast<int>(count - 2) - static_cast<int>(_focus.index()), count);
                        invalidate();
                    }
                    break;
                case Page::designVehicles:
                {
                    if (choice >= _designVehicles.size())
                    {
                        page(_designVehicleFromFile ? Page::designInspect : Page::designOptions);
                        break;
                    }
                    const auto* td = _designVehicleFromFile ? _designMetadata.get() : Handheld::Designs::Draft();
                    if (!td || _designVehicleContext != TrackDesignPlacementContext::Capture(*td)
                        || (_designVehicleFromFile && (!_designFile || !Handheld::FileBrowsing::Current(*_designFile))))
                    {
                        _status = "Objects or file changed. Back and choose again.";
                        invalidate();
                        break;
                    }
                    const auto chosen = _designVehicles[choice];
                    if (_designVehicleFromFile)
                    {
                        _designChoice = chosen;
                        _designFilePreview = Handheld::Designs::FileEntry(_designFile->path.u8string(), chosen, 260, 144);
                        page(Page::designInspect);
                    }
                    else if (Handheld::Designs::ChooseVehicle(chosen))
                        page(Page::designOptions);
                    else
                    {
                        _status = "Vehicle unavailable. Back and choose again.";
                        invalidate();
                    }
                    break;
                }
                case Page::designInspect:
                    if (choice == 1)
                        page(Page::designFiles);
                    else if (choice == 2)
                    {
                        openDesignVehicles(true);
                    }
                    else if (choice == 3)
                    {
                        _designDetailsFromFile = true;
                        _designDetail = 0;
                        page(Page::designWarnings);
                    }
                    else if (!_designFile || !Handheld::FileBrowsing::Current(*_designFile))
                    {
                        _status = "Design file changed. Back and select again.";
                        invalidate();
                    }
                    else if (!fileDesignEligible())
                    {
                        _status = _designFileStatus;
                        invalidate();
                    }
                    else if (Handheld::BeginSavedDesign(_designFile->path.u8string(), _designChoice))
                    {
                        Handheld::Designs::SetBrowserReturn(true);
                        close();
                    }
                    else
                    {
                        _status = "Placement unavailable. Back and inspect again.";
                        invalidate();
                    }
                    break;
                case Page::parkFiles:
                {
                    if (choice == 0)
                    {
                        const auto previous = _files.directory;
                        _files.Parent();
                        _fileFocus = 0;
                        for (size_t i = 0; i < _files.entries.size(); ++i)
                            if (_files.entries[i].path == previous)
                                _fileFocus = i + Handheld::FileBrowsing::kFirst;
                        _fileDetail = 0;
                        page(Page::parkFiles);
                    }
                    else if (choice - Handheld::FileBrowsing::kFirst < _files.entries.size())
                    {
                        const auto entry = _files.entries[choice - Handheld::FileBrowsing::kFirst];
                        if (entry.directory)
                        {
                            _files.Open(entry.path, _files.pattern);
                            _fileFocus = 0;
                            _fileDetail = 0;
                            page(Page::parkFiles);
                        }
                        else
                        {
                            _loadFile = Handheld::FileBrowsing::Capture(entry.path);
                            if (!_loadFile)
                            {
                                _status = "File unavailable. Open the folder again.";
                                invalidate();
                            }
                            else
                            {
                                _parkFilePreview.reset();
                                _parkPreviewStatus = "Preview unavailable for this format";
                                if (ParkImporter::ExtensionIsOpenRCT2ParkFile(_loadFile->path.extension().u8string()))
                                {
                                    try
                                    {
                                        auto importer = ParkImporter::CreateParkFile(GetContext()->GetObjectRepository());
                                        auto stream = FileStream(_loadFile->path.u8string(), FileMode::open);
                                        importer->LoadFromStream(&stream, false, true, _loadFile->path.u8string());
                                        _parkFilePreview = importer->GetParkPreview();
                                        _parkPreviewStatus = _parkFilePreview->parkName.empty()
                                            ? "This file has no park summary"
                                            : "";
                                    }
                                    catch (const std::exception&)
                                    {
                                        _parkPreviewStatus = "Preview unavailable; try loading the park";
                                    }
                                }
                                page(Page::parkInspect);
                            }
                        }
                    }
                    break;
                }
                case Page::load:
                {
                    if (choice == 3)
                    {
                        openParkFiles();
                        break;
                    }
                    _fileLoad = false;
                    _slot = choice;
                    _loadFile = Handheld::FileBrowsing::Capture(SlotPath(_slot));
                    std::error_code error;
                    if (!fs::exists(SlotPath(_slot), error))
                    {
                        _status = "This slot is empty.";
                        invalidate();
                    }
                    else if (gLegacyScene == LegacyScene::playing)
                        page(Page::discard);
                    else
                        load();
                    break;
                }
                case Page::overwrite:
                    if (choice == 0)
                        save();
                    else
                        page(Page::save);
                    break;
                case Page::discard:
                    if (choice == 0)
                        load();
                    else
                        page(_fileLoad ? Page::parkFiles : Page::load);
                    break;
                case Page::quitChoice:
                    if (choice == 0)
                    {
                        _exitAfterSave = true;
                        _quitSaving = true;
                        page(Page::save);
                    }
                    else if (choice == 1)
                        page(Page::exit);
                    else
                        cancelQuit();
                    break;
                case Page::exit:
                    if (choice == 0)
                    {
                        close();
                        // The deliberate Pocket discard confirmation is final.
                        Handheld::Resume::Discard();
                        GetContext()->Finish();
                    }
                    else
                    {
                        page(Page::quitChoice);
                        _focus = {};
                        _focus.move(1, 3);
                    }
                    break;
            }
        }
        void onMouseUp(WidgetIndex widget) override
        {
            if (widget == 2)
                back();
        }
        void onDraw(RenderTarget& screen) override
        {
            Handheld::MenuThemes::settings.Load(GetContext()->GetPlatformEnvironment().GetDirectoryPath(DirBase::user));
            Handheld::Theme::Refresh();
            Handheld::ApplyAccent(accent());
            playFeelSound(_menuFeel.Observe(
                scrollKey(), _focus.index() + (_dashboard.index() << 16), Handheld::MenuFeel::Clock(),
                static_cast<int>(_page)));
            // Page transitions offset a copy of the target, so every draw below slides together.
            RenderTarget rt = screen;
            const auto [slideX, slideY] = _menuFeel.Slide();
            rt.x -= slideX;
            rt.y -= slideY;
            rt.cullingX -= slideX;
            rt.cullingY -= slideY;
            Handheld::MenuFeelScope buttonMotion(_menuFeel);
            Handheld::FooterSurfaceScope fixedFooter(screen);
            _menuFeel.base = { reinterpret_cast<const uint8_t*>(rt.bits), rt.x, rt.y, rt.LineStride() };
            // Runs after every early return below, drawing the pointer above all content.
            struct PointerOverlay
            {
                RenderTarget& target;
                Handheld::MenuFeel& feel;
                ~PointerOverlay()
                {
                    feel.EndFrame();
                    Handheld::DrawFocusPointer(target, feel);
                }
            } pointerOverlay{ rt, _menuFeel };
            Handheld::ListScrollScope listMotion(_scrollViews[scrollKey()]);
            if (_page == Page::staffUniform && _staffTarget)
            {
                auto model = staffPresentation();
                model.breadcrumb = "Staff / Uniform colour";
                Handheld::DrawManagementContent(rt, { 8, 36 }, { 248, 312 }, model);
                Handheld::DrawFooter(
                    rt, { ContextGetWidth(), ContextGetHeight() },
                    { { "D-pad", "Colour" }, { "A", "Apply to role" }, { "B", "Cancel" } });
                return;
            }
            if (_page == Page::accessManage || _page == Page::accessPoint)
            {
                drawAccessSelection(rt);
                return;
            }

            if (_page == Page::trackBlocks || _page == Page::trackBlock || _page == Page::trackContext)
            {
                drawTrackSelection(rt);
                return;
            }
            if (_page == Page::build)
            {
                Handheld::DrawChoiceGroups(
                    rt, { ContextGetWidth(), ContextGetHeight() }, "Build", choices(),
                    { { "BUILD", 104 }, { "SHAPE THE LAND", 70 }, { "TOOLS", 30 } }, _focus.index(), "Park");
                return;
            }
            if (choiceCards())
            {
                Handheld::DrawChoiceCards(
                    rt, { ContextGetWidth(), ContextGetHeight() },
                    _page == Page::build              ? "Build your park"
                        : _page == Page::paths        ? "Paths & queues"
                        : _page == Page::customChoice ? "Build this ride"
                                                      : "Scenery",
                    choices(), _focus.index(), _page == Page::build ? 4 : 3, _page == Page::build ? "Park" : "Back");
                return;
            }
            if (_page == Page::settings)
            {
                // Grouped toggles, with what the focused one does spelled out underneath.
                using namespace Handheld;
                const ScreenSize canvas{ ContextGetWidth(), ContextGetHeight() };
                const int w = canvas.width - 16, h = Layout::ContentHeight(canvas);
                DrawSurface(rt, { 8, 8 }, { w, h });
                DrawHeadingText(rt, { 20, 19 }, "{WHITE}Settings", w - 24);
                Fill(rt, { 20, 45 }, { w - 24, 1 }, Theme::edge);
                const auto& prefs = Handheld::preferences;
                struct Row
                {
                    const char* section;
                    const char* label;
                    bool on;
                    std::string value;
                    const char* help;
                };
                const std::array<Row, 7> list{ {
                    { "DISPLAY", "Theme", false, MenuThemes::Choice().substr(7), "Daylight, Slate or the classic Pixel look." },
                    { nullptr, "Motion", false, MenuThemes::MotionChoice().substr(8),
                      "Reduced turns off slides, glides and scrolling names." },
                    { "PLAYING",
                      "Smart cursor",
                      prefs.smartCursor,
                      {},
                      "The cursor slows over rides, stalls and guests so it is easier to stop on them." },
                    { "HUD",
                      "Goal in the HUD",
                      prefs.hudGoal,
                      {},
                      "Shows your scenario goal's progress under the money and date." },
                    { nullptr,
                      "Clock and battery in the HUD",
                      prefs.hudClock,
                      {},
                      "Shows the time and battery level in the top bar." },
                    { "DEVICE",
                      "Continue last park on launch",
                      prefs.resumeOnLaunch,
                      {},
                      "Skips the title screen and opens the park you played last." },
                    { nullptr,
                      "Save when the device sleeps",
                      prefs.saveOnSleep,
                      {},
                      "Autosaves when the device sleeps or the game is closed, so nothing is lost." },
                } };
                int y = 52;
                const auto focused = _focus.index();
                for (size_t i = 0; i < list.size(); ++i)
                {
                    if (list[i].section)
                    {
                        DrawSectionLabel(rt, { 22, y + 4 }, w - 28, list[i].section);
                        y += 18;
                    }
                    DrawToggleRow(rt, { 16, y }, w - 16, list[i].label, list[i].on, i == focused, list[i].value);
                    y += 28;
                }
                if (focused < list.size())
                    DrawModernEllipsised(rt, { 20, h - 10 }, w - 24, "{MUTED}" + std::string(list[focused].help));
                if (!_status.empty())
                    DrawModernEllipsised(rt, { 20, h - 24 }, w - 24, "{CAUTION}" + _status);
                Handheld::DrawFooter(rt, canvas, { { "D-pad", "Choose" }, { "A", "Change" }, { "B", "Back" } });
                return;
            }
            if (_page == Page::controls)
            {
                Handheld::Guide::DrawPage(rt, { ContextGetWidth(), ContextGetHeight() }, _focus.index());
                return;
            }
            if (_page == Page::primer)
            {
                Handheld::Guide::DrawIntro(rt, { ContextGetWidth(), ContextGetHeight() }, _focus.index());
                return;
            }
            if (_leaveAction)
            {
                Handheld::DrawManagementContent(rt, { 80, 32 }, { 352, 292 }, leavePresentation());
                Handheld::DrawFooter(
                    rt, { ContextGetWidth(), ContextGetHeight() },
                    { { "D-pad", "Choose" }, { "A", _civicBusy ? "Wait" : "Select" }, { "B", _civicBusy ? "Wait" : "Edit" } });
                return;
            }
            if (_page == Page::staffRoles || _page == Page::staffCostumes)
            {
                auto model = staffPresentation();
                std::optional<ObjectEntryIndex> costume;
                const auto row = _focus.index();
                auto role = _page == Page::staffRoles ? static_cast<StaffType>(std::min<size_t>(row, 3))
                                                      : StaffType::entertainer;
                const auto costumes = Handheld::StaffManagement::Costumes();
                const bool preview = _page == Page::staffRoles ? row < 4 : row < costumes.size();
                if (_page == Page::staffCostumes && preview)
                    costume = costumes[row].first;
                if (_page == Page::staffRoles && preview)
                    model.stats = { { "Monthly wage", Handheld::FormatMoney(GetStaffWage(role)) } };
                Handheld::DrawManagementContent(rt, { 8, 36 }, { 312, 312 }, model);
                Handheld::Concept::Box(rt, { 328, 36 }, { 176, 312 });
                if (preview)
                {
                    Handheld::DrawWrappedLabel(
                        rt, { 340, 56 }, 152,
                        _page == Page::staffRoles ? Handheld::StaffManagement::RoleName(role) : costumes[row].second, 4);
                    Handheld::StaffManagement::DrawSpritePreview(rt, { 416, 192 }, role, costume);
                }
                Handheld::DrawFooter(
                    rt, { ContextGetWidth(), ContextGetHeight() },
                    { { "D-pad", "Choose" }, { "A", "Select" }, { "B", "Back" } });
                return;
            }
            if (rideVehicleChoice())
            {
                const auto picker = rideVehiclePicker();
                Handheld::DrawObjectPicker(
                    rt, { ContextGetWidth(), ContextGetHeight() }, picker, "Ride vehicles", "A: Review train changes",
                    "No vehicles available");
                return;
            }
            if (_page == Page::designScenery)
            {
                const auto selected = _focus.index();
                if (selected >= 3 && selected - 3 < Handheld::DesignAuthoring::sceneryBaseline.size())
                {
                    const auto& element = Handheld::DesignAuthoring::sceneryBaseline[selected - 3];
                    if (auto* main = WindowGetMain(); main && main->viewport)
                    {
                        auto& view = *main->viewport;
                        main->savedViewPos = Translate3DTo2DWithZ(view.rotation, element.loc)
                            - ScreenCoordsXY{ view.zoom.ApplyTo(384 - view.pos.x), view.zoom.ApplyTo(192 - view.pos.y) };
                        view.viewPos = main->savedViewPos;
                        main->flags.unset(WindowFlag::scrollingToLocation);
                        const auto selectionFlags = gMapSelectFlags;
                        const auto type = gMapSelectType;
                        const auto a = gMapSelectPositionA, b = gMapSelectPositionB;
                        gMapSelectFlags = { MapSelectFlag::enable };
                        gMapSelectType = MapSelectType::full;
                        gMapSelectPositionA = gMapSelectPositionB = CoordsXY(element.loc);
                        ViewportRender(rt, main->viewport);
                        gMapSelectFlags = selectionFlags;
                        gMapSelectType = type;
                        gMapSelectPositionA = a;
                        gMapSelectPositionB = b;
                    }
                }
                auto model = ridesPresentationFor(_page, selected);
                model.breadcrumb = "Save design / Scenery";
                Handheld::DrawManagementContent(rt, { 8, 36 }, { 240, 312 }, model);
                Handheld::DrawFooter(
                    rt, { ContextGetWidth(), ContextGetHeight() },
                    { { "D-pad", "Choose" }, { "A", "Toggle / select" }, { "B", "Back" } });
                return;
            }
            if (_page == Page::save || _page == Page::load)
            {
                // Save slots as cards (picture, name, date, how the park is doing), then Browse.
                const ScreenSize canvas{ ContextGetWidth(), ContextGetHeight() };
                const ScreenCoordsXY p{ Handheld::Layout::margin, Handheld::Layout::margin };
                const int w = canvas.width - 2 * Handheld::Layout::margin, h = Handheld::Layout::ContentHeight(canvas);
                const bool saving = _page == Page::save;
                Handheld::DrawSurface(rt, p, { w, h });
                Handheld::DrawHeadingText(
                    rt, p + ScreenCoordsXY{ 12, 11 }, saving ? "{WHITE}Save park" : "{WHITE}Load park", w - 24);
                Handheld::Fill(rt, p + ScreenCoordsXY{ 12, 37 }, { w - 24, 1 }, Handheld::Theme::edge);
                const int cardH = std::clamp((h - 46 - 46 - 12) / 3, 52, 70);
                int y = 46;
                for (size_t slot = 0; slot < 3; ++slot)
                {
                    std::error_code error;
                    Handheld::SavedParkPresentation::DrawSlot(
                        rt, p + ScreenCoordsXY{ 12, y }, { w - 24, cardH }, slot, _slotPreviews[slot],
                        fs::exists(SlotPath(slot), error), _focus.index() == slot, saving);
                    y += cardH + 6;
                }
                Handheld::DrawActionRow(
                    rt, p + ScreenCoordsXY{ 12, y + 4 }, { w - 24, 30 },
                    saving ? "Save with a name..." : "Browse parks and autosaves...", "", _focus.index() == 3, true,
                    Handheld::PanelIcon::folder);
                if (!_status.empty())
                    Handheld::DrawModernEllipsised(rt, p + ScreenCoordsXY{ 12, h - 16 }, w - 24, "{WHITE}" + _status);
                Handheld::DrawFooter(rt, canvas, { { "D-pad", "Choose" }, { "A", saving ? "Save" : "Load" }, { "B", "Back" } });
                return;
            }
            if (_page == Page::parkInspect || _page == Page::designInspect)
            {
                const bool slots = false;
                const bool design = _page == Page::designInspect;
                Handheld::ManagementPresentation model;
                model.rows = rows();
                model.selected = _focus.index();
                model.rowHeight = 38;
                model.title = slots ? (_page == Page::save ? "Save park" : "Load park")
                    : design        ? "Saved design"
                                    : "Saved park";
                model.status = _status;
                if (!slots)
                {
                    const auto filename = design ? (_designFile ? _designFile->path.filename().u8string() : "")
                                                 : (_loadFile ? _loadFile->path.filename().u8string() : "");
                    model.bodyLines = { filename };
                    model.rowHeight = 30;
                }
                Handheld::DrawManagementContent(rt, { 8, 36 }, { 232, 312 }, model);
                Handheld::Concept::Box(rt, { 248, 36 }, { 256, 312 });
                const auto* park = slots
                    ? (_focus.index() < 3 && _slotPreviews[_focus.index()] ? &*_slotPreviews[_focus.index()] : nullptr)
                    : _parkFilePreview ? &*_parkFilePreview
                                       : nullptr;
                if (!design && park)
                    Handheld::SavedParkPresentation::Draw(rt, *park);
                else if (design)
                {
                    Handheld::DrawWrappedLabel(
                        rt, { 258, 50 }, 236, _designFile ? _designFile->path.stem().u8string() : "File unavailable", 2);
                    Drawing::RenderTarget image;
                    if (_designFilePreview && _designFilePreview->drawPreview
                        && ClipRenderTarget(image, rt, { 246, 82 }, 260, 144))
                        _designFilePreview->drawPreview(image);
                    const auto* object = _designMetadata
                        ? GetContext()->GetObjectManager().GetLoadedObject(
                              _designChoice.value_or(_designMetadata->trackAndVehicle.vehicleObject))
                        : nullptr;
                    Handheld::DrawWrappedLabel(rt, { 258, 236 }, 236, object ? object->GetName() : "Choose another vehicle", 2);
                    const auto readiness = fileDesignEligible() ? "Ready / choose land for cost"
                        : _designMetadata                       ? Handheld::Designs::UnavailableReason(*_designMetadata)
                                                                : _designFileStatus;
                    Handheld::DrawWrappedLabel(rt, { 258, 286 }, 236, readiness, 4);
                }
                else
                    Handheld::DrawWrappedLabel(
                        rt, { 258, 154 }, 236, slots ? "Choose a saved park or Browse." : _parkPreviewStatus, 4);
                Handheld::DrawFooter(
                    rt, { ContextGetWidth(), ContextGetHeight() },
                    { { "D-pad", "Choose" }, { "A", "Select" }, { "B", "Back" } });
                return;
            }
            if (_page == Page::researchCatalog || _page == Page::designVehicles)
            {
                const bool research = _page == Page::researchCatalog;
                const auto picker = research ? Handheld::ResearchCatalog::Picker(_researchCatalog, _focus.index())
                                             : designVehiclePicker();
                Handheld::DrawObjectPicker(
                    rt, { ContextGetWidth(), ContextGetHeight() }, picker, research ? "Research catalog" : "Choose vehicle",
                    research ? "Y: " + Handheld::ResearchCatalog::Category(_researchFilter) : "Compatible vehicles",
                    research ? "No researched items" : "No compatible vehicles");
                // The full-screen picker has its own detail pane (name, preview, detail line); the old
                // overlay box drew on top of it, and of its empty state.
                Handheld::DrawFooter(
                    rt, { ContextGetWidth(), ContextGetHeight() },
                    { { "D-pad", "Choose" },
                      { "A", research ? "Details" : "Use vehicle" },
                      { "Y", research ? "Category" : "" },
                      { "B", "Back" } });
                return;
            }
            if (_page == Page::designOptions)
            {
                if (!_designWorkspacePreview)
                    refreshDesignWorkspace();
                Handheld::ManagementPresentation model;
                model.title = "Place saved design";
                model.rows = rows();
                model.selected = _focus.index();
                model.rowHeight = 30;
                model.status = _status;
                Handheld::DrawManagementContent(rt, { 8, 36 }, { 232, 312 }, model);
                Handheld::Concept::Box(rt, { 248, 36 }, { 256, 312 });
                Drawing::RenderTarget image;
                if (_designWorkspacePreview && _designWorkspacePreview->drawPreview
                    && ClipRenderTarget(image, rt, { 246, 82 }, 260, 144))
                    _designWorkspacePreview->drawPreview(image);
                Handheld::DrawWrappedLabel(
                    rt, { 258, 50 }, 236,
                    _designWorkspacePreview ? _designWorkspacePreview->name
                                            : fs::u8path(Handheld::Designs::Selection()).stem().u8string(),
                    2);
                const auto* draft = Handheld::Designs::Draft();
                const auto* vehicle = draft
                    ? GetContext()->GetObjectManager().GetLoadedObject(draft->trackAndVehicle.vehicleObject)
                    : nullptr;
                Handheld::DrawWrappedLabel(rt, { 258, 232 }, 236, vehicle ? vehicle->GetName() : "Vehicle unavailable", 2);
                Handheld::DrawModernText(
                    rt, { 258, 274 },
                    _designQuote == kMoney64Undefined ? "Choose land for cost" : "Cost " + Handheld::FormatMoney(_designQuote),
                    {}, 236);
                Handheld::DrawWrappedLabel(rt, { 258, 296 }, 236, _designReadiness, 3);
                Handheld::DrawFooter(
                    rt, { ContextGetWidth(), ContextGetHeight() },
                    { { "D-pad", "Choose / adjust" }, { "A", "Select" }, { "B", "Placement" } });
                return;
            }
            if (_page == Page::designWarnings && !_designTechnical)
            {
                const auto* td = _designDetailsFromFile ? _designMetadata.get() : Handheld::Designs::Draft();
                std::string vehicle = "Vehicle unavailable", state = "Choose another vehicle", scenery = "Design unavailable";
                const Object* vehicleObject = nullptr;
                bool sceneryCanRemain = false;
                if (td)
                {
                    const auto descriptor = _designDetailsFromFile ? _designChoice.value_or(td->trackAndVehicle.vehicleObject)
                                                                   : td->trackAndVehicle.vehicleObject;
                    const auto* object = GetContext()->GetObjectManager().GetLoadedObject(descriptor);
                    if (object)
                        vehicle = object->GetName();
                    vehicleObject = object;
                    state = "Vehicle: " + Handheld::DesignAvailability::State(descriptor);
                    const auto availability = Handheld::DesignAvailability::Lines(*td);
                    const bool missing = std::any_of(availability.begin(), availability.end(), [](const auto& row) {
                        return row.starts_with("Missing scenery:");
                    });
                    for (const auto& line : availability)
                        if (line == "Ride type: unavailable" || line == "No loaded station style for placement")
                            state = line;
                    sceneryCanRemain = !td->sceneryElements.empty() && !missing
                        && (_designDetailsFromFile || Handheld::Designs::IncludesScenery());
                    scenery = td->sceneryElements.empty()                                  ? "No scenery in this design"
                        : !_designDetailsFromFile && !Handheld::Designs::IncludesScenery() ? "Scenery is switched off"
                        : missing ? "Missing objects: ALL design scenery will be left out"
                                  : "Scenery included; see Details for individual objects";
                    if (!_designDetailsFromFile)
                        for (const auto& line : Handheld::Designs::WarningLines())
                            if (line.starts_with("Scenery omitted:"))
                                scenery = line;
                }
                Handheld::DrawDesignCheck(
                    rt, vehicle, state, scenery,
                    _status.empty() ? (_designDetailsFromFile ? std::string{} : Handheld::Designs::LastError()) : _status,
                    _focus.index(), vehicleObject, sceneryCanRemain);
                return;
            }
            if (_page == Page::designVehicles || _page == Page::designWarnings || _page == Page::designConfirm)
            {
                Handheld::ManagementPresentation model;
                model.breadcrumb = "Designs / Options";
                model.title = _page == Page::designVehicles ? "Choose vehicle"
                    : _page == Page::designWarnings         ? "Availability and warnings"
                                                            : "Place saved design?";
                model.rows = rows();
                model.selected = _focus.index();
                if (_page == Page::designConfirm)
                {
                    model.confirmation = true;
                    model.bodyLines = Handheld::Designs::WarningLines();
                    model.stats = { { "Cost", Handheld::FormatMoney(Handheld::Designs::ReviewCost()) } };
                    model.stats.push_back({ "Placement", Handheld::Designs::HeightNotice() });
                }
                else
                {
                    if (_page == Page::designWarnings && model.rows.size() >= 2)
                        model.rowReadOnly = std::vector<bool>(model.rows.size() - 2, true);
                    const auto labels = rows();
                    const auto text = _focus.index() < labels.size() ? labels[_focus.index()] : std::string{};
                    model.bodyLines = { Handheld::FileBrowsing::Detail(text, _designDetail),
                                        "Left/Right: full selected detail" };
                    if (_page == Page::designVehicles && _designVehicles.empty())
                        model.status = "No compatible researched vehicles available";
                }
                if (!_status.empty())
                    model.status = _status;
                Handheld::DrawManagementContent(rt, { 8, 8 }, { 496, SheetHeight() }, model);
                std::vector<Handheld::ControlHint> hints{ { "D-pad", "Read" } };
                if (_page == Page::designWarnings)
                    hints.push_back({ "Left/Right", "Detail" });
                if (_page == Page::designConfirm || model.selected + 2 >= model.rows.size())
                    hints.push_back({ "A", "Select" });
                hints.push_back({ "B", "Back" });
                Handheld::DrawFooter(rt, { ContextGetWidth(), ContextGetHeight() }, hints);
                return;
            }
            if (_page == Page::parkInspect || _page == Page::designInspect)
            {
                Handheld::ManagementPresentation model;
                model.rows = rows();
                model.selected = _focus.index();
                if (_page == Page::parkInspect)
                {
                    model.breadcrumb = "Load / Park information";
                    model.title = _loadFile ? _loadFile->path.filename().u8string() : "File unavailable";
                    if (_parkFilePreview && !_parkFilePreview->parkName.empty())
                    {
                        const auto& p = *_parkFilePreview;
                        model.bodyLines = { p.parkName };
                        model.stats = { { "Rating", std::to_string(p.parkRating) },
                                        { "Guests", std::to_string(p.numGuests) },
                                        { "Rides", std::to_string(p.numRides) } };
                    }
                    model.status = _status.empty() ? _parkPreviewStatus : _status;
                }
                else
                {
                    model.breadcrumb = "Designs / Design information";
                    model.title = _designFile ? _designFile->path.filename().u8string() : "File unavailable";
                    if (_designMetadata)
                    {
                        const auto& d = *_designMetadata;
                        model.stats = { { "Required vehicle", std::string(d.trackAndVehicle.vehicleObject.GetName()) },
                                        { "Track / maze",
                                          std::to_string(d.trackElements.size()) + " / "
                                              + std::to_string(d.mazeElements.size()) },
                                        { "Scenery", std::to_string(d.sceneryElements.size()) },
                                        { "Format", trackDesignGetExtension(d.version) } };
                    }
                    if (_focus.index() > 2)
                    {
                        model.stats.clear();
                        const auto labels = rows();
                        const auto text = _focus.index() < labels.size() ? labels[_focus.index()] : std::string{};
                        model.bodyLines = { Handheld::FileBrowsing::Detail(text, _designDetail),
                                            "Left/Right: full selected detail" };
                    }
                    else
                        model.bodyLines = { "Design details", "", "", "" };
                    model.status = _status.empty()
                        ? (fileDesignEligible()
                               ? (_designChoice ? "Selected vehicle: " + _designChoice->ToString() : "Original vehicle")
                               : _designMetadata ? Handheld::Designs::UnavailableReason(*_designMetadata)
                                                 : _designFileStatus)
                        : _status;
                    if (_designFile && !Handheld::FileBrowsing::Current(*_designFile))
                        model.status = "File changed. Back and select again.";
                }
                Handheld::DrawManagementContent(rt, { 8, 8 }, { 496, SheetHeight() }, model);
                if (_page == Page::designInspect && _focus.index() <= 2 && _designFilePreview
                    && _designFilePreview->drawPreview)
                {
                    Drawing::RenderTarget preview;
                    if (ClipRenderTarget(preview, rt, { 420, 166 }, 60, 62))
                        _designFilePreview->drawPreview(preview);
                }
                Handheld::Concept::Box(rt, { 8, ContextGetHeight() - 25 }, { 496, 25 });
                Handheld::DrawModernText(rt, { 16, ContextGetHeight() - 15 }, "D-pad Select   A Choose   B Back", {}, 480);
                return;
            }
            if ((_page == Page::parkFiles || _page == Page::saveFiles || _page == Page::designFiles))
            {
                // Files as a list you would see on a console: friendly names, what each one is and
                // when it was saved, folders marked as folders, and the folder's name (not its path)
                // in the header.
                namespace FB = Handheld::FileBrowsing;
                Handheld::ManagementPresentation model;
                model.breadcrumb = std::string(
                                       _page == Page::designFiles     ? "Designs / "
                                           : _page == Page::saveFiles ? "Save / "
                                                                      : "Load / ")
                    + _files.directory.u8string();
                model.title = _page == Page::designFiles ? "Track designs"
                    : _page == Page::saveFiles           ? "Choose where to save"
                                                         : "Browse parks";
                const auto folder = _files.directory.filename().u8string();
                model.headerStatus = folder == "save" ? "Saves"
                    : folder == "autosave"            ? "Autosaves"
                    : folder == "track"               ? "Designs"
                    : folder.empty()                  ? _files.directory.u8string()
                                                      : folder;
                model.selected = _focus.index();
                model.denseRoster = true;
                const size_t lead = _page == Page::saveFiles ? 1 : 0;
                if (lead)
                {
                    model.rows.push_back("Save in this folder...");
                    model.rowDetails.push_back("Name the park on the next screen");
                }
                model.rows.push_back("Up one folder");
                model.rowDetails.push_back(_files.directory.parent_path().filename().u8string());
                for (const auto& entry : _files.entries)
                {
                    model.rows.push_back(FB::DisplayName(entry.path, entry.directory));
                    const auto when = entry.directory ? std::string{} : FB::Modified(entry.path);
                    const auto kind = entry.directory ? std::string("Folder")
                        : _page == Page::designFiles  ? std::string("Track design")
                                                      : std::string("Saved park");
                    model.rowDetails.push_back(when.empty() ? kind : kind + " / " + when);
                }
                model.status = !_status.empty() ? _status
                    : !_files.error.empty()     ? _files.error
                    : _files.entries.empty()    ? "No parks or folders here"
                                                : "";
                const ScreenSize canvas{ ContextGetWidth(), ContextGetHeight() };
                Handheld::DrawManagementContent(
                    rt, { 8, 8 }, { canvas.width - 16, Handheld::Layout::ContentHeight(canvas) }, model);
                Handheld::DrawFooter(rt, canvas, { { "D-pad", "Choose" }, { "A", "Open" }, { "B", "Back" } });
                return;
            }
            if (_page == Page::saveReview)
            {
                Handheld::ManagementPresentation model;
                model.breadcrumb = "Save As / Review";
                model.title = _saveDraft ? _saveDraft->destination.filename().u8string() : "Review expired";
                model.confirmation = true;
                model.rows = rows();
                model.selected = _focus.index();
                model.bodyLines = { _saveDraft ? _saveDraft->directory.u8string() : _files.directory.u8string() };
                model.status = _status.empty() ? "Previous save kept until publication succeeds" : _status;
                Handheld::DrawManagementContent(rt, { 8, 8 }, { 496, SheetHeight() }, model);
                Handheld::Concept::Box(rt, { 8, ContextGetHeight() - 25 }, { 496, 25 });
                Handheld::DrawModernText(rt, { 16, ContextGetHeight() - 15 }, "D-pad Select   A Choose   B Cancel", {}, 480);
                return;
            }
            if (_page == Page::saveName || _page == Page::scenerySearch)
            {
                auto model = Handheld::StaffManagement::NamePresentation(_nameEntry, _focus.index(), false, _status);
                model.breadcrumb = _page == Page::saveName ? "Save As / Filename" : "Scenery / Search";
                model.title = _page == Page::saveName ? "Park filename" : "All eligible objects";
                model.keyboardHeading = _page == Page::saveName ? "Save park as" : "Search scenery";
                model.status = _status;
                Handheld::DrawManagementContent(
                    rt, { 8, 8 },
                    { ContextGetWidth() - 16, Handheld::Layout::ContentHeight({ ContextGetWidth(), ContextGetHeight() }) },
                    model);
                Handheld::DrawFooter(rt, { ContextGetWidth(), ContextGetHeight() }, Handheld::KeyboardHints());
                return;
            }
            if (_page == Page::parkMap)
            {
                Handheld::ParkMap::Draw(rt, _parkMap);
                return;
            }
            if (parkInformation())
            {
                auto model = parkInformationPresentation();
                if (_page == Page::parkHistory)
                    model.bodyLines.assign(10, "");
                if (_page == Page::financeHistory)
                    model.bodyLines.assign(8, "");
                Handheld::DrawManagementContent(rt, { 8, 8 }, { 496, SheetHeight() }, model);
                if (_page == Page::parkHistory)
                    Handheld::ParkInformation::DrawHistory(rt, _parkGuestHistory);
                if (_page == Page::financeHistory)
                    Handheld::FinanceInformation::DrawHistory(rt, _financeGraph);
                if (_page == Page::researchItem && _researchItem)
                    Handheld::ResearchCatalog::DrawPreview(rt, *_researchItem);
                Handheld::DrawFooter(
                    rt, { ContextGetWidth(), ContextGetHeight() },
                    { { "D-pad", "Choose" }, { "A", "Select" }, { "B", "Back" } });
                return;
            }
            if (managementShell())
            {
                // One builder for a section's content, used both for the page itself and for the
                // rail's preview of it, so the preview always matches the page it leads to.
                auto build = [&]() {
                    Handheld::ManagementPresentation model;
                    if (overviewManagement())
                        model = overviewPresentation();
                    else if (ridesManagement())
                        model = ridesPresentationFor(_page, _focus.index());
                    else if (guestManagement())
                        model = guestsPresentation();
                    else if (staffManagement())
                        model = staffPresentation();
                    else if (researchManagement())
                        model = researchPresentation();
                    else if (marketingManagement())
                        model = marketingPresentationFor(_page, _focus.index());
                    else
                    {
                        using Handheld::Finances::View;
                        model = _page == Page::finances ? financeWorkspace()
                                                        : Handheld::Finances::Presentation(
                                                              _page == Page::finances            ? View::overview
                                                                  : _page == Page::financeEditor ? View::editor
                                                                                                 : View::confirmation,
                                                              _focus.index(), _financeDraft, _status);
                    }
                    return model;
                };
                Handheld::ManagementPresentation model;
                if (_page == Page::dashboard)
                {
                    // Preview: the page entering the section would open (where you left it, or the
                    // section's first page), built as that page, then the rail state put back.
                    using Handheld::Dashboard::Section;
                    const auto root = _dashboard.section == Section::staff ? std::optional<Page>{ Page::staff }
                        : _dashboard.section == Section::finances          ? std::optional<Page>{ Page::finances }
                        : _dashboard.section == Section::research          ? std::optional<Page>{ Page::research }
                        : _dashboard.section == Section::marketing         ? std::optional<Page>{ Page::marketing }
                        : _dashboard.section == Section::guests            ? std::optional<Page>{ Page::guests }
                        : _dashboard.section == Section::rides             ? std::optional<Page>{ Page::ridesRoster }
                        : _dashboard.section == Section::overview          ? std::optional<Page>{ Page::overview }
                                                                           : std::nullopt;
                    if (!root || !_dashboard.category().available)
                        model = Handheld::Dashboard::Unavailable(_dashboard.section);
                    else
                    {
                        const auto resume = _sectionResume[_dashboard.index()];
                        const auto target = resume.page.value_or(*root);
                        // The lists a page normally fills when it is opened.
                        if (target == Page::ridesRoster)
                            _ridesRoster = Handheld::RideManagement::Roster();
                        if (target == Page::staff)
                            _staffRoster = Handheld::StaffManagement::Roster(_staffFilter);
                        if (target == Page::guestRoster)
                            _guestRoster = Handheld::GuestManagement::Roster(_guestFilter);
                        if (target == Page::marketing)
                            _marketingActiveTypes = Handheld::MarketingManagement::ActiveTypes();
                        const auto railPage = _page;
                        const auto railFocus = _focus;
                        _page = target;
                        _focus = {};
                        if (const auto count = rows().size(); count && resume.page)
                            _focus.move(static_cast<int32_t>(std::min(resume.row, count - 1)), count);
                        model = build();
                        _page = railPage;
                        _focus = railFocus;
                    }
                    if (!_status.empty())
                        model.status = _status;
                }
                else
                    model = build();
                Handheld::Dashboard::Draw(
                    rt, { ContextGetWidth(), ContextGetHeight() }, _dashboard, model, dashboardCanSwitch());
                if (_page == Page::finances && _financeTabs.tab == 2)
                    Handheld::FinanceInformation::DrawHistory(
                        rt, _financeGraph, { { 104, 158 }, { ContextGetWidth() - 24, 254 } },
                        static_cast<int32_t>(_financeSample));
                return;
            }
            if (_page == Page::shopAppearance || _page == Page::shopAppearanceEditor || _page == Page::shopAppearanceReview)
            {
                Handheld::DrawManagementContent(rt, { 8, 8 }, { 330, SheetHeight() }, shopAppearancePresentation());
                Handheld::DrawActionStrip(rt, { ContextGetWidth(), ContextGetHeight() });
                Handheld::DrawFooter(
                    rt, { ContextGetWidth(), ContextGetHeight() },
                    { { "D-pad", "Choose" },
                      { "A", _page == Page::shopAppearanceEditor ? "Apply" : "Select" },
                      { "B", "Back" } });
                return;
            }
            if (_page == Page::shop)
            {
                const auto model = shopPresentation();
                Handheld::DrawManagementContent(rt, { 8, 8 }, { 330, SheetHeight() }, model);
                Handheld::DrawActionStrip(rt, { ContextGetWidth(), ContextGetHeight() });
                const auto row = _focus.index();
                const bool onPrice = row >= 1 && row <= shopPriceCount();
                using Hints = std::vector<Handheld::ControlHint>;
                Handheld::DrawFooter(
                    rt, { ContextGetWidth(), ContextGetHeight() },
                    row == 0      ? Hints{ { "D-pad", "Open / close" }, { "A", "Switch" }, { "B", "Back" } }
                        : onPrice ? Hints{ { "D-pad", "Price" }, { "B", "Back" } }
                                  : Hints{ { "D-pad", "Choose" }, { "A", "Select" }, { "B", "Back" } });
                return;
            }
            if (_page == Page::shopDemolish)
            {
                // The same review card as ride demolition: what you get back, Cancel focused first.
                Handheld::ManagementPresentation model;
                const auto* ride = _shop ? Handheld::ShopManagement::Find(*_shop) : nullptr;
                model.title = ride ? ride->getName() : "Shop unavailable";
                model.confirmation = true;
                model.destructive = true;
                model.selected = _focus.index();
                model.status = _status;
                model.rows = { "Demolish shop", "Cancel" };
                if (ride)
                {
                    const auto quote = Handheld::ShopManagement::Demolish(*_shop, false);
                    const bool noMoney = getGameState().park.flags.has(ParkFlag::noMoney);
                    model.stats = { { "Removes", "This building" },
                                    { "Refund",
                                      quote.error != GameActions::Status::ok ? quote.getErrorMessage()
                                          : noMoney                          ? std::string("None")
                                                                             : Handheld::FormatMoney(-quote.cost) } };
                }
                Handheld::DrawManagementContent(rt, { 8, 8 }, { 330, SheetHeight() }, model);
                Handheld::DrawActionStrip(rt, { ContextGetWidth(), ContextGetHeight() });
                Handheld::DrawFooter(
                    rt, { ContextGetWidth(), ContextGetHeight() },
                    { { "D-pad", "Choose" }, { "A", "Select" }, { "B", "Cancel" } });
                return;
            }
            if (_page == Page::trackPiecePicker)
            {
                Handheld::DrawObjectPicker(
                    rt, { ContextGetWidth(), ContextGetHeight() }, _picker, "Track pieces",
                    _status.empty() ? "Y Group: " + Handheld::CustomTracks::GroupName(Handheld::CustomTracks::CurrentGroup())
                                    : _status,
                    "No compatible pieces", false, "Ride", "Camera");
                return;
            }
            if (_page == Page::designPicker)
            {
                Handheld::DrawObjectPicker(
                    rt, { ContextGetWidth(), ContextGetHeight() }, _picker, "Saved designs",
                    _status.empty() ? "X Browse files" : _status, "No supported designs installed");
                return;
            }
            if (_page == Page::ridePicker)
            {
                static const std::vector<std::string> tabs{ "All", "Coasters", "Thrill", "Gentle", "Water", "Transport" };
                Handheld::DrawObjectPicker(
                    rt, { ContextGetWidth(), ContextGetHeight() }, _picker, "Rides", _status, "None of this type available yet",
                    false, "Pause", "", "Type", tabs, _rideCategory % kRideCategories.size());
                return;
            }
            if (_page == Page::shopPicker)
            {
                Handheld::DrawObjectPicker(
                    rt, { ContextGetWidth(), ContextGetHeight() }, _picker, "Shops / Facilities", _status,
                    "No shops available");
                return;
            }
            if (_page == Page::peepCancel)
            {
                Handheld::DrawManagementPanel(
                    rt, { ContextGetWidth(), ContextGetHeight() }, Handheld::PeepPlacement::CancelPresentation(_focus.index()));
                return;
            }
            if (_page == Page::staffPatrolClear && _patrolDraft)
            {
                Handheld::DrawManagementPanel(
                    rt, { ContextGetWidth(), ContextGetHeight() },
                    Handheld::StaffPatrol::Presentation(*_patrolDraft, _focus.index()));
                return;
            }
            if (_page == Page::rightsConfirm && _rightsDraft)
            {
                Handheld::DrawManagementPanel(
                    rt, { ContextGetWidth(), ContextGetHeight() },
                    Handheld::RightsTools::Presentation(*_rightsDraft, _focus.index(), _status));
                return;
            }
            if (_page == Page::terrainPaintConfirm && _paintDraft)
            {
                Handheld::DrawManagementPanel(
                    rt, { ContextGetWidth(), ContextGetHeight() },
                    Handheld::AreaTools::PaintPresentation(*_paintDraft, _focus.index(), _status));
                return;
            }
            if (_page == Page::sceneryClearConfirm && _clearDraft)
            {
                Handheld::DrawManagementPanel(
                    rt, { ContextGetWidth(), ContextGetHeight() },
                    Handheld::SceneryClear::Presentation(*_clearDraft, _focus.index(), _status));
                return;
            }
            if (_page == Page::sceneryStyleEditor || _page == Page::sceneryStyleConfirm)
            {
                Handheld::ManagementPresentation model;
                model.breadcrumb = "Scenery / Sign style";
                model.title = _signStyle ? Handheld::SceneryTools::StyleValue(*_signStyle) : "Sign unavailable";
                model.rows = rows();
                model.selected = _focus.index();
                model.confirmation = _page == Page::sceneryStyleConfirm;
                model.status = _status.empty() ? "No charge to change this style" : _status;
                Handheld::DrawManagementPanel(rt, { ContextGetWidth(), ContextGetHeight() }, model);
                return;
            }
            if (_page == Page::sceneryName || _page == Page::sceneryConfirm)
            {
                Handheld::ManagementPresentation model;
                model.breadcrumb = "Scenery / Sign text";
                model.title = _nameEntry.text.empty() ? "Empty text" : _nameEntry.text;
                model.rows = rows();
                model.selected = _focus.index();
                model.columns = _page == Page::sceneryName ? Handheld::TextEntry::columns : 1;
                model.confirmation = _page == Page::sceneryConfirm;
                if (_page == Page::sceneryName)
                {
                    model.keyboard = _nameEntry;
                    model.keyboardHeading = "Sign text";
                    model.title = "Empty sign";
                }
                model.status = _status;
                Handheld::DrawManagementContent(
                    rt, Handheld::Concept::kMenuOrigin,
                    { 324, Handheld::Layout::ContentHeight({ ContextGetWidth(), ContextGetHeight() }) }, model);
                Handheld::DrawActionStrip(rt, { ContextGetWidth(), ContextGetHeight() });
                Handheld::DrawFooter(
                    rt, { ContextGetWidth(), ContextGetHeight() },
                    _page == Page::sceneryName
                        ? Handheld::KeyboardHints()
                        : std::vector<Handheld::ControlHint>{ { "D-pad", "Choose" }, { "A", "Select" }, { "B", "Back" } });
                return;
            }
            if (_page == Page::sceneryPicker)
            {
                Handheld::DrawObjectPicker(
                    rt, { ContextGetWidth(), ContextGetHeight() }, _picker,
                    Handheld::SceneryTools::CategoryName(Handheld::SceneryTools::Current().category),
                    _status.empty() ? Handheld::SceneryTools::GroupName()
                            + (Handheld::SceneryTools::Search().empty() ? "" : " / " + Handheld::SceneryTools::Search())
                                    : _status,
                    "No matching available objects", true);
                return;
            }
            if (_page == Page::treePicker)
            {
                Handheld::DrawObjectPicker(
                    rt, { ContextGetWidth(), ContextGetHeight() }, _picker, "Choose tree", _status, "No trees available");
                return;
            }
            if (_page == Page::pathUndoConfirm && _pathUndo)
            {
                Handheld::ManagementPresentation model;
                model.breadcrumb = "Build / Connected path";
                model.title = "Remove last new section?";
                model.rows = rows();
                model.selected = _focus.index();
                model.confirmation = true;
                model.stats = { { "Cost / refund", Handheld::FormatMoney(_pathUndo->cost) } };
                model.status = _status.empty() ? "Existing paths preserved" : _status;
                Handheld::DrawManagementPanel(rt, { ContextGetWidth(), ContextGetHeight() }, model);
                return;
            }
            if (_page == Page::terrainMaterialPicker)
            {
                Handheld::DrawObjectPicker(
                    rt, { ContextGetWidth(), ContextGetHeight() }, _picker,
                    _terrainMaterialEdge ? "Terrain edges" : "Terrain surfaces", _status, "No loaded materials");
                return;
            }
            if (_page == Page::pathSurfacePicker)
            {
                Handheld::DrawObjectPicker(
                    rt, { ContextGetWidth(), ContextGetHeight() }, _picker,
                    Handheld::Paths::IsQueue() ? "Queue surfaces" : "Path surfaces", _status, "No eligible surfaces");
                return;
            }
            if (_page == Page::pathOptions)
            {
                Handheld::DrawPathOptions(
                    rt, { ContextGetWidth(), ContextGetHeight() }, rows(), _focus.index(),
                    _status.empty() ? Handheld::Paths::PlacementName() : _status);
                return;
            }
            if (_page == Page::sceneryOptions || _page == Page::areaOptions)
            {
                Handheld::DrawToolSettings(
                    rt, { ContextGetWidth(), ContextGetHeight() }, _page == Page::sceneryOptions, rows(), _focus.index(),
                    _status);
                return;
            }
            if (_page == Page::park)
            {
                const auto hud = Handheld::NativeHud();
                const bool paused = (gGamePaused & ~(_pausedHere ? GAME_PAUSED_NORMAL : 0)) != 0;
                Handheld::PauseSummary summary{ getGameState().park.name, hud.date, hud.noMoney ? "" : hud.cash,
                                                hud.guests + " guests",
                                                paused ? "Paused" : std::to_string(1u << (gGameSpeed - 1)) + "x speed" };
                Handheld::DrawPauseScreen(
                    rt, { ContextGetWidth(), ContextGetHeight() }, rows(), _focus.index(), summary, _status);
                Handheld::DrawFooter(
                    rt, { ContextGetWidth(), ContextGetHeight() },
                    { { "D-pad", "Choose" }, { "A", "Select" }, { "B", "Resume" } });
                return;
            }
            const char* title = "Paused";
            switch (_page)
            {
                case Page::dashboard:
                    title = "Park dashboard";
                    break;
                case Page::overview:
                    title = "Park overview";
                    break;
                case Page::finances:
                    title = "Finances";
                    break;
                case Page::guests:
                    title = "Guests";
                    break;
                case Page::research:
                    title = "Research";
                    break;
                case Page::ride:
                    title = "Ride";
                    break;
                case Page::paths:
                    title = "Paths";
                    break;
                case Page::pathOptions:
                    title = "Path options";
                    break;
                case Page::customChoice:
                    title = "Build a ride";
                    break;
                case Page::designOptions:
                    title = "Design options";
                    break;
                case Page::designConfirm:
                    title = "Place design?";
                    break;
                case Page::peepOptions:
                case Page::peepCancel:
                    title = "Place person";
                    break;
                case Page::staffPatrolOptions:
                    title = "Staff patrol";
                    break;
                case Page::rightsOptions:
                    title = "Land and rights";
                    break;
                case Page::areaOptions:
                    title = "Brush options";
                    break;
                case Page::sceneryClearOptions:
                    title = "Clear area";
                    break;
                case Page::sceneryGroups:
                    title = "Scenery group";
                    break;
                case Page::sceneryStyle:
                    title = "Sign style";
                    break;
                case Page::sceneryCategory:
                    title = "Scenery";
                    break;
                case Page::sceneryOptions:
                    title = "Scenery options";
                    break;
                case Page::build:
                    title = "Build";
                    break;
                case Page::save:
                    title = _exitAfterSave ? "Save and quit" : "Save park";
                    break;
                case Page::load:
                    title = "Load park";
                    break;
                case Page::overwrite:
                    title = "Replace save?";
                    break;
                case Page::discard:
                    title = "Discard unsaved changes?";
                    break;
                case Page::quitChoice:
                    title = "Quit game";
                    break;
                case Page::exit:
                    title = "Exit without saving?";
                    break;
                default:
                    break;
            }
            Handheld::MenuPresentation model;
            model.title = title;
            model.destructive = _page == Page::exit || _page == Page::discard || _page == Page::overwrite;
            model.rows = rows();
            model.selected = _focus.index();
            model.status = _status;
            if (_page == Page::sceneryClearOptions)
                model.icons = { Handheld::PanelIcon::play,  Handheld::PanelIcon::none,         Handheld::PanelIcon::none,
                                Handheld::PanelIcon::tree,  Handheld::PanelIcon::largeScenery, Handheld::PanelIcon::path,
                                Handheld::PanelIcon::walls, Handheld::PanelIcon::furniture };
            if (_page == Page::pathOptions)
            {
                model.icons = { Handheld::PanelIcon::play,  Handheld::PanelIcon::terrain, Handheld::PanelIcon::raise,
                                Handheld::PanelIcon::lower, Handheld::PanelIcon::path,    Handheld::PanelIcon::path,
                                Handheld::PanelIcon::path };
                if (model.status.empty())
                    model.status = Handheld::Paths::PlacementName();
            }
            if (_page == Page::park)
            {
                model.icons = { Handheld::PanelIcon::play, Handheld::PanelIcon::terrain, Handheld::PanelIcon::folder,
                                Handheld::PanelIcon::save, Handheld::PanelIcon::exit,    Handheld::PanelIcon::folder,
                                Handheld::PanelIcon::exit };
                if (model.status.empty())
                    model.status = "Simulation: "
                        + std::string((gGamePaused & ~(_pausedHere ? GAME_PAUSED_NORMAL : 0)) != 0
                                          ? "Paused"
                                          : (std::to_string(1u << (gGameSpeed - 1)) + "x"));
            }
            else if (_page == Page::build)
            {
                model.icons = { Handheld::PanelIcon::raise,  Handheld::PanelIcon::lower,        Handheld::PanelIcon::path,
                                Handheld::PanelIcon::none,   Handheld::PanelIcon::terrain,      Handheld::PanelIcon::remove,
                                Handheld::PanelIcon::tree,   Handheld::PanelIcon::none,         Handheld::PanelIcon::none,
                                Handheld::PanelIcon::folder, Handheld::PanelIcon::largeScenery, Handheld::PanelIcon::terrain,
                                Handheld::PanelIcon::none,   Handheld::PanelIcon::back };
                if (model.status.empty())
                    model.status = "Choose a tool, then edit in the park.";
            }
            // Never replace a save failure with the confirmation's normal hint.
            if (model.status.empty())
            {
                if (_page == Page::discard || _page == Page::exit)
                    model.status = "Unsaved changes will be lost.";
                else if (_page == Page::overwrite)
                    model.status = "This replaces the selected saved park.";
            }
            if (information())
            {
                const auto& state = getGameState();
                const auto& park = state.park;
                model.detailTitle = park.name;
                switch (_page)
                {
                    case Page::overview:
                        model.stats = { { "Guests", std::to_string(park.numGuestsInPark) },
                                        { "Rating", std::to_string(park.rating) },
                                        { "Park value", money(park.value) },
                                        { "Company value", money(park.companyValue) },
                                        { "Cash", money(park.cash) } };
                        break;
                    case Page::finances:
                        model.stats = { { "Cash", money(park.cash) },
                                        { "Bank loan", money(park.bankLoan) },
                                        { "Current profit", money(park.currentProfit) },
                                        { "Expenditure", money(park.currentExpenditure) },
                                        { "Park value", money(park.value) } };
                        break;
                    case Page::guests:
                        model.stats = { { "In park", std::to_string(park.numGuestsInPark) },
                                        { "Arriving", std::to_string(park.numGuestsHeadingForPark) },
                                        { "Total admissions", std::to_string(park.totalAdmissions) },
                                        { "Park rating", std::to_string(park.rating) } };
                        break;
                    case Page::research:
                    {
                        static constexpr const char* funding[] = { "None", "Minimum", "Normal", "Maximum" };
                        model.stats = { { "Funding", funding[std::min<int>(state.researchFundingLevel, 3)] },
                                        { "Stage progress",
                                          std::to_string(static_cast<uint32_t>(state.researchProgress) * 100 / 65536) + "%" },
                                        { "Stage",
                                          state.researchProgressStage == 0       ? "Initial research"
                                              : state.researchProgressStage == 1 ? "Designing"
                                              : state.researchProgressStage == 2 ? "Completing design"
                                                                                 : "Complete" } };
                        break;
                    }
                    case Page::ride:
                        if (const auto* ride = GetRide(_rideId))
                        {
                            model.detailTitle = ride->getName();
                            model.stats = { { "Status",
                                              ride->status == RideStatus::open         ? "Open"
                                                  : ride->status == RideStatus::closed ? "Closed"
                                                                                       : "Testing" },
                                            { "Riders", std::to_string(ride->numRiders) },
                                            { "Reliability", std::to_string(ride->reliabilityPercentage) + "%" },
                                            { "Profit", money(ride->profit) } };
                        }
                        else
                            model.status = "This ride is no longer available.";
                        break;
                    default:
                        break;
                }
                Handheld::DrawInformationOverlay(rt, *this, model);
            }
            else
                Handheld::DrawOverlayShell(rt, *this, model);
        }
    };

    static HandheldMenuWindow* OpenPanel()
    {
        auto* wm = GetWindowManager();
        auto* window = wm->BringToFrontByClass(WindowClass::handheldMenu);
        if (window == nullptr)
            window = wm->Create<HandheldMenuWindow>(
                WindowClass::handheldMenu, {}, kSize, { WindowFlag::transparent, WindowFlag::noBackground });
        return static_cast<HandheldMenuWindow*>(window);
    }
    void HandheldMenuOpen()
    {
        OpenPanel();
    }
    WindowBase* HandheldQuitOpen()
    {
        const bool standalone = GetWindowManager()->FindByClass(WindowClass::handheldMenu) == nullptr;
        auto* window = OpenPanel();
        if (window)
            window->openQuit(standalone);
        return window;
    }
    void HandheldBuildOpen()
    {
        if (auto* window = OpenPanel())
            window->openBuild();
    }
    void HandheldAccessReviewOpen(const Handheld::RideAccess::EditDraft& draft)
    {
        if (auto* window = OpenPanel())
            window->openAccessReview(draft);
    }
    void HandheldRideAccessOpen(const Handheld::RideManagement::Target& target)
    {
        if (auto* window = OpenPanel())
            window->openRideAccess(target);
    }
    void HandheldPathSurfacePickerOpen()
    {
        if (auto* window = OpenPanel())
            window->openPathSurfacePicker(true);
    }
    void HandheldPathOptionsOpen()
    {
        if (auto* window = OpenPanel())
            window->openPathOptions();
    }
    void HandheldLoadOpen()
    {
        if (auto* window = OpenPanel())
            window->openLoad();
    }
    Handheld::ManagementPresentation HandheldShopAppearancePresentation()
    {
        auto* window = GetWindowManager()->FindByClass(WindowClass::handheldMenu);
        return window ? static_cast<HandheldMenuWindow*>(window)->shopAppearancePresentation()
                      : Handheld::ManagementPresentation{};
    }
    void HandheldMenuMove(int32_t direction)
    {
        if (auto* window = GetWindowManager()->FindByClass(WindowClass::handheldMenu))
            static_cast<HandheldMenuWindow*>(window)->move(direction);
    }
    bool HandheldTextEntryKey(int action)
    {
        auto* window = GetWindowManager()->FindByClass(WindowClass::handheldMenu);
        return window && static_cast<HandheldMenuWindow*>(window)->textEntryKey(static_cast<Handheld::Action>(action));
    }
    void HandheldMenuConfirm()
    {
        if (auto* window = GetWindowManager()->FindByClass(WindowClass::handheldMenu))
            static_cast<HandheldMenuWindow*>(window)->confirm();
    }
    void HandheldMenuBack()
    {
        if (auto* window = GetWindowManager()->FindByClass(WindowClass::handheldMenu))
            static_cast<HandheldMenuWindow*>(window)->back();
    }
    void HandheldRideReturnNotice(std::string notice)
    {
        sRideReturnNotice = std::move(notice);
    }
    void HandheldRideReturn(
        const Handheld::RideManagement::Target& target, const std::optional<Handheld::RideNavigation::Context>& context)
    {
        if (auto* window = OpenPanel())
            window->returnRide(target, context);
    }
    bool HandheldRideQuickAccess()
    {
        auto* window = GetWindowManager()->FindByClass(WindowClass::handheldMenu);
        return window && static_cast<HandheldMenuWindow*>(window)->quickRideAccess();
    }
    void HandheldRideOpen(RideId ride)
    {
        if (auto* window = OpenPanel())
            window->openRide(ride);
    }
} // namespace OpenRCT2::Ui::Windows

namespace OpenRCT2::Ui::Windows
{
    bool HandheldShopManagementIsOpen()
    {
        const auto* window = GetWindowManager()->FindByClass(WindowClass::handheldMenu);
        return window && static_cast<const HandheldMenuWindow*>(window)->shopIsOpen();
    }
    void HandheldShopManagementMove(int x, int y)
    {
        if (auto* window = GetWindowManager()->FindByClass(WindowClass::handheldMenu))
            static_cast<HandheldMenuWindow*>(window)->shopMove(x, y);
    }
    void HandheldShopManagementPause()
    {
        if (auto* window = GetWindowManager()->FindByClass(WindowClass::handheldMenu))
            static_cast<HandheldMenuWindow*>(window)->shopPause();
    }
    void HandheldDesignPickerOpen()
    {
        if (auto* window = OpenPanel())
            window->openDesignPicker(!Handheld::Designs::Active());
    }
    void HandheldDesignReviewPause()
    {
        if (auto* window = GetWindowManager()->FindByClass(WindowClass::handheldMenu))
            static_cast<HandheldMenuWindow*>(window)->pauseDesignReview();
    }
    void HandheldDesignOptionsOpen()
    {
        if (auto* window = OpenPanel())
            window->openDesignOptions();
    }
    void HandheldDesignReviewOpen()
    {
        if (auto* window = OpenPanel())
            window->openDesignReview();
    }
    void HandheldRidePickerOpen()
    {
        if (auto* window = OpenPanel())
            window->openRidePicker(Handheld::FlatRides::CurrentStage() != Handheld::FlatRides::Stage::ride);
    }
    void HandheldShopPickerOpen()
    {
        if (auto* window = OpenPanel())
            window->openShopPicker(!Handheld::IsShopToolActive());
    }
    bool HandheldObjectPickerIsOpen()
    {
        const auto* window = GetWindowManager()->FindByClass(WindowClass::handheldMenu);
        return window && static_cast<const HandheldMenuWindow*>(window)->picker();
    }
    void HandheldObjectPickerMove(int x, int y)
    {
        if (auto* window = GetWindowManager()->FindByClass(WindowClass::handheldMenu))
            static_cast<HandheldMenuWindow*>(window)->pickerMove(x, y);
    }
    void HandheldObjectPickerPause()
    {
        if (auto* window = GetWindowManager()->FindByClass(WindowClass::handheldMenu))
            static_cast<HandheldMenuWindow*>(window)->pickerPause();
    }
    ObjectEntryIndex HandheldObjectPickerFocus()
    {
        const auto* window = GetWindowManager()->FindByClass(WindowClass::handheldMenu);
        return window && static_cast<const HandheldMenuWindow*>(window)->picker()
            ? static_cast<const HandheldMenuWindow*>(window)->pickerFocus()
            : kObjectEntryIndexNull;
    }
    Handheld::ManagementPresentation HandheldParkMapSubjectsPresentation()
    {
        const auto* w = static_cast<HandheldMenuWindow*>(GetWindowManager()->FindByClass(WindowClass::handheldMenu));
        return w ? w->mapSubjectsModel() : Handheld::ManagementPresentation{};
    }
    std::optional<CoordsXY> HandheldParkMapLocation()
    {
        const auto* w = static_cast<HandheldMenuWindow*>(GetWindowManager()->FindByClass(WindowClass::handheldMenu));
        return w && w->parkMapOpen() ? std::optional<CoordsXY>{ w->mapFocus() } : std::nullopt;
    }
    void HandheldParkMapSubjects()
    {
        if (auto* w = static_cast<HandheldMenuWindow*>(GetWindowManager()->FindByClass(WindowClass::handheldMenu)))
            w->parkMapSubjects();
    }
    bool HandheldParkMapIsOpen()
    {
        auto* w = static_cast<HandheldMenuWindow*>(GetWindowManager()->FindByClass(WindowClass::handheldMenu));
        return w && w->parkMapOpen();
    }
    void HandheldParkMapControl(int control)
    {
        if (auto* w = static_cast<HandheldMenuWindow*>(GetWindowManager()->FindByClass(WindowClass::handheldMenu)))
            w->parkMapControl(control);
    }
    bool HandheldConstructionMenuIsOpen()
    {
        const auto* w = static_cast<HandheldMenuWindow*>(GetWindowManager()->FindByClass(WindowClass::handheldMenu));
        return w && w->constructionMenu();
    }
    void HandheldConstructionMove(int x, int y)
    {
        if (auto* w = static_cast<HandheldMenuWindow*>(GetWindowManager()->FindByClass(WindowClass::handheldMenu)))
            w->constructionMove(x, y);
    }
    void HandheldRightsOptionsOpen()
    {
        if (auto* w = OpenPanel())
            w->openRightsOptions();
    }
    void HandheldRightsReviewOpen(CoordsXY tile)
    {
        if (auto* w = OpenPanel())
            w->openRightsReview(tile);
    }
    void HandheldTerrainPaintReviewOpen(CoordsXY tile)
    {
        if (auto* window = OpenPanel())
            window->openTerrainPaintReview(tile);
    }
    void HandheldAreaOptionsOpen()
    {
        if (auto* w = OpenPanel())
            w->openAreaOptions();
    }
    void HandheldSceneryClearOptionsOpen()
    {
        if (auto* w = OpenPanel())
            w->openSceneryClearOptions();
    }
    void HandheldSceneryClearReviewOpen(CoordsXY tile)
    {
        if (auto* w = OpenPanel())
            w->openSceneryClearReview(tile);
    }
    void HandheldSceneryStyleOpen(const Handheld::SceneryTools::Target& target)
    {
        if (auto* window = OpenPanel())
            window->openSceneryStyle(target);
    }
    void HandheldSceneryCatalogOpen()
    {
        if (auto* window = OpenPanel())
            window->openSceneryCatalog();
    }
    void HandheldTerrainMaterialPickerOpen(bool edge)
    {
        if (auto* window = OpenPanel())
            window->openTerrainMaterialPicker(edge);
    }
    void HandheldSceneryOptionsOpen()
    {
        if (auto* window = OpenPanel())
            window->openSceneryOptions();
    }
    bool HandheldSceneryPickerIsOpen()
    {
        const auto* window = GetWindowManager()->FindByClass(WindowClass::handheldMenu);
        return window && static_cast<const HandheldMenuWindow*>(window)->sceneryPicker();
    }
    void HandheldScenerySearchOpen()
    {
        if (auto* window = OpenPanel())
            window->openScenerySearch();
    }
    void HandheldTreePickerOpen()
    {
        if (!Handheld::IsTreeToolActive())
            return;
        if (auto* window = OpenPanel())
            window->openTreePicker();
    }
    bool HandheldTreePickerIsOpen()
    {
        const auto* window = GetWindowManager()->FindByClass(WindowClass::handheldMenu);
        return window && static_cast<const HandheldMenuWindow*>(window)->treePicker();
    }
    void HandheldTreePickerMove(int x, int y)
    {
        HandheldObjectPickerMove(x, y);
    }
    void HandheldTreePickerPause()
    {
        HandheldObjectPickerPause();
    }
    ObjectEntryIndex HandheldTreePickerFocus()
    {
        const auto* window = GetWindowManager()->FindByClass(WindowClass::handheldMenu);
        return window && static_cast<const HandheldMenuWindow*>(window)->treePicker()
            ? static_cast<const HandheldMenuWindow*>(window)->pickerFocus()
            : kObjectEntryIndexNull;
    }
} // namespace OpenRCT2::Ui::Windows

namespace OpenRCT2::Ui::Windows
{
    std::string HandheldNewsText(const std::string& text)
    {
        return Handheld::ParkInformation::PlainText(text);
    }
    bool HandheldControlsIntro()
    {
        auto* w = GetWindowManager()->FindByClass(WindowClass::handheldMenu);
        return w && static_cast<HandheldMenuWindow*>(w)->controlsIntro();
    }
    void HandheldPrimerOpen()
    {
        HandheldDashboardOpen();
        auto* w = GetWindowManager()->FindByClass(WindowClass::handheldMenu);
        if (w)
            static_cast<HandheldMenuWindow*>(w)->openPrimer(false);
    }
    void HandheldInboxOpen()
    {
        HandheldDashboardOpen();
        auto* w = GetWindowManager()->FindByClass(WindowClass::handheldMenu);
        if (w)
            static_cast<HandheldMenuWindow*>(w)->openInbox();
    }
    void HandheldNewsOpen()
    {
        HandheldDashboardOpen();
        auto* w = GetWindowManager()->FindByClass(WindowClass::handheldMenu);
        if (w)
            static_cast<HandheldMenuWindow*>(w)->openNews(true);
    }
    void HandheldObjectivesOpen()
    {
        HandheldDashboardOpen();
        auto* w = GetWindowManager()->FindByClass(WindowClass::handheldMenu);
        if (w)
            static_cast<HandheldMenuWindow*>(w)->openObjectives();
    }
    bool HandheldDashboardNews()
    {
        auto* w = GetWindowManager()->FindByClass(WindowClass::handheldMenu);
        return w && static_cast<HandheldMenuWindow*>(w)->dashboardNews();
    }
    bool HandheldToolSettingsPage(int direction)
    {
        auto* w = GetWindowManager()->FindByClass(WindowClass::handheldMenu);
        return w && static_cast<HandheldMenuWindow*>(w)->toolSettingsPage(direction);
    }
    bool HandheldTrackSelectionMove(int x, int y)
    {
        auto* w = GetWindowManager()->FindByClass(WindowClass::handheldMenu);
        return w && static_cast<HandheldMenuWindow*>(w)->trackSelectionMove(x, y);
    }
    bool HandheldObjectPickerGroup()
    {
        auto* w = GetWindowManager()->FindByClass(WindowClass::handheldMenu);
        return w && static_cast<HandheldMenuWindow*>(w)->pickerGroup();
    }
    bool HandheldFinancesIsOpen()
    {
        const auto* window = GetWindowManager()->FindByClass(WindowClass::handheldMenu);
        return window && static_cast<const HandheldMenuWindow*>(window)->financeIsOpen();
    }
    void HandheldFinancesMove(int x, int y)
    {
        if (auto* window = GetWindowManager()->FindByClass(WindowClass::handheldMenu))
            static_cast<HandheldMenuWindow*>(window)->financeMove(x, y);
    }
    void HandheldFinancesPause()
    {
        if (auto* window = GetWindowManager()->FindByClass(WindowClass::handheldMenu))
            static_cast<HandheldMenuWindow*>(window)->financePause();
    }
} // namespace OpenRCT2::Ui::Windows

namespace OpenRCT2::Ui::Windows
{
    void HandheldStaffRemoved(EntityId id)
    {
        Handheld::StaffManagement::Invalidate(id);
        if (auto* window = GetWindowManager()->FindByClass(WindowClass::handheldMenu))
            static_cast<HandheldMenuWindow*>(window)->staffRemoved(id);
    }
    void HandheldStaffPatrolOptionsOpen()
    {
        if (auto* window = OpenPanel())
            window->openStaffPatrolOptions();
    }
    void HandheldGuestOpen(const Handheld::GuestManagement::Target& person)
    {
        auto* window = OpenPanel();
        if (window)
            window->openGuest(person);
    }
    void HandheldPeepPlacementOptionsOpen()
    {
        auto* window = OpenPanel();
        if (window)
            window->openPeepOptions();
    }
    void HandheldStaffOpen(std::optional<Handheld::StaffManagement::Target> target)
    {
        if (auto* window = OpenPanel())
            window->openStaff(target);
    }
    Handheld::ManagementPresentation HandheldStaffPresentation()
    {
        const auto* window = GetWindowManager()->FindByClass(WindowClass::handheldMenu);
        return window ? static_cast<const HandheldMenuWindow*>(window)->staffModel() : Handheld::ManagementPresentation{};
    }
    bool HandheldStaffIsOpen()
    {
        const auto* window = GetWindowManager()->FindByClass(WindowClass::handheldMenu);
        return window && static_cast<const HandheldMenuWindow*>(window)->staffIsOpen();
    }
    void HandheldStaffPause()
    {
        if (auto* window = GetWindowManager()->FindByClass(WindowClass::handheldMenu))
            static_cast<HandheldMenuWindow*>(window)->staffPause();
    }
} // namespace OpenRCT2::Ui::Windows

namespace OpenRCT2::Ui::Windows
{
    void HandheldDashboardPause()
    {
        if (auto* window = GetWindowManager()->FindByClass(WindowClass::handheldMenu))
            static_cast<HandheldMenuWindow*>(window)->dashboardPause();
    }
    void HandheldSignEditOpen()
    {
        if (auto* window = OpenPanel())
            window->openSignFromPark();
    }
    void HandheldQuickJump(Handheld::QuickWheel::Target target)
    {
        if (auto* window = OpenPanel())
            window->quickJump(target);
    }
    std::pair<std::string, uint32_t> HandheldLastBuild()
    {
        return { sLastBuildLabel, sLastBuildSprite };
    }
    void HandheldDashboardOpen()
    {
        if (auto* window = OpenPanel())
            window->openDashboard();
    }
    bool HandheldDashboardIsOpen()
    {
        const auto* window = GetWindowManager()->FindByClass(WindowClass::handheldMenu);
        return window && static_cast<const HandheldMenuWindow*>(window)->dashboardIsOpen();
    }
    bool HandheldDashboardRailFocused()
    {
        const auto* window = GetWindowManager()->FindByClass(WindowClass::handheldMenu);
        return window && static_cast<const HandheldMenuWindow*>(window)->dashboardRailFocused();
    }
    void HandheldDashboardMove(int x, int y)
    {
        if (auto* window = GetWindowManager()->FindByClass(WindowClass::handheldMenu))
            static_cast<HandheldMenuWindow*>(window)->dashboardMove(x, y);
    }
    void HandheldDashboardPage(int direction)
    {
        if (auto* window = GetWindowManager()->FindByClass(WindowClass::handheldMenu))
            static_cast<HandheldMenuWindow*>(window)->dashboardPage(direction);
    }
    void HandheldDashboardSwitchPane()
    {
        if (auto* window = GetWindowManager()->FindByClass(WindowClass::handheldMenu))
            static_cast<HandheldMenuWindow*>(window)->dashboardSwitchPane();
    }
} // namespace OpenRCT2::Ui::Windows

namespace OpenRCT2::Ui::Windows
{
    size_t HandheldDashboardSectionIndex()
    {
        auto* window = GetWindowManager()->FindByClass(WindowClass::handheldMenu);
        return window ? static_cast<const HandheldMenuWindow*>(window)->dashboardSectionIndex() : 0;
    }
    bool HandheldOverviewIsOpen()
    {
        auto* window = GetWindowManager()->FindByClass(WindowClass::handheldMenu);
        return window && static_cast<const HandheldMenuWindow*>(window)->overviewIsOpen();
    }
    Handheld::ManagementPresentation HandheldOverviewPresentation()
    {
        auto* window = GetWindowManager()->FindByClass(WindowClass::handheldMenu);
        return window ? static_cast<const HandheldMenuWindow*>(window)->overviewModel() : Handheld::ManagementPresentation{};
    }
    bool HandheldRidesIsOpen()
    {
        auto* window = GetWindowManager()->FindByClass(WindowClass::handheldMenu);
        return window && static_cast<const HandheldMenuWindow*>(window)->ridesIsOpen();
    }
    Handheld::ManagementPresentation HandheldRidesPresentation()
    {
        auto* window = GetWindowManager()->FindByClass(WindowClass::handheldMenu);
        return window ? static_cast<const HandheldMenuWindow*>(window)->ridesPresentation()
                      : Handheld::ManagementPresentation{};
    }
    bool HandheldGuestsIsOpen()
    {
        const auto* window = GetWindowManager()->FindByClass(WindowClass::handheldMenu);
        return window && static_cast<const HandheldMenuWindow*>(window)->guestsIsOpen();
    }
    Handheld::ManagementPresentation HandheldGuestsPresentation()
    {
        const auto* window = GetWindowManager()->FindByClass(WindowClass::handheldMenu);
        return window ? static_cast<const HandheldMenuWindow*>(window)->guestsPresentation()
                      : Handheld::ManagementPresentation{};
    }
} // namespace OpenRCT2::Ui::Windows

namespace OpenRCT2::Ui::Windows
{
    void HandheldTrackPiecePickerOpen()
    {
        if (auto* window = OpenPanel())
            window->openTrackPiecePicker();
    }
} // namespace OpenRCT2::Ui::Windows

namespace OpenRCT2::Ui::Windows
{
    bool HandheldDesignPickerIsOpen()
    {
        const auto* w = static_cast<const HandheldMenuWindow*>(GetWindowManager()->FindByClass(WindowClass::handheldMenu));
        return w && w->designPickerOpen();
    }
    void HandheldDesignFilesOpen()
    {
        if (auto* w = OpenPanel())
            w->openDesignFiles(!Handheld::Designs::Active());
    }
} // namespace OpenRCT2::Ui::Windows

namespace OpenRCT2::Ui::Windows
{
    Handheld::ManagementPresentation HandheldManagementPresentation()
    {
        const auto* w = GetWindowManager()->FindByClass(WindowClass::handheldMenu);
        return w ? static_cast<const HandheldMenuWindow*>(w)->managementPresentation() : Handheld::ManagementPresentation{};
    }
    bool HandheldManagementContext()
    {
        auto* w = GetWindowManager()->FindByClass(WindowClass::handheldMenu);
        return w && static_cast<HandheldMenuWindow*>(w)->managementContext();
    }
} // namespace OpenRCT2::Ui::Windows

namespace OpenRCT2::Ui::Windows
{
    bool HandheldObjectPickerCamera()
    {
        if (auto* window = GetWindowManager()->FindByClass(WindowClass::handheldMenu))
            return static_cast<HandheldMenuWindow*>(window)->pickerCamera();
        return false;
    }
} // namespace OpenRCT2::Ui::Windows
