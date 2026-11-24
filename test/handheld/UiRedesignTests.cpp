// SPDX-License-Identifier: GPL-3.0-only
#include "RideTabNavigation.h"

#include <SDL.h>
#include <cstring>
#include <filesystem>
#include <iostream>
#include <openrct2-ui/UiContext.h>
#include <openrct2-ui/handheld/ControllerInput.h>
#include <openrct2-ui/handheld/Explorer.h>
#include <openrct2-ui/handheld/PathSurfacePicker.h>
#include <openrct2-ui/handheld/RideTabs.h>
#include <openrct2-ui/interface/Window.h>
#include <openrct2-ui/windows/Windows.h>
#include <openrct2/Context.h>
#include <openrct2/GameState.h>
#include <openrct2/Input.h>
#include <openrct2/OpenRCT2.h>
#include <openrct2/PlatformEnvironment.h>
#include <openrct2/actions/GameActionRunner.h>
#include <openrct2/audio/AudioContext.h>
#include <openrct2/config/Config.h>
#include <openrct2/interface/Viewport.h>
#include <openrct2/interface/WindowBase.h>
#include <openrct2/object/ObjectLimits.h>
#include <openrct2/scenario/Scenario.h>
#include <openrct2/scenario/ScenarioCategory.h>
#include <openrct2/scenario/ScenarioRepository.h>
#include <openrct2/scenario/ScenarioSources.h>
#include <openrct2/ui/UiContext.h>
#include <openrct2/ui/WindowManager.h>
#include <openrct2/world/Map.h>
#include <openrct2/world/tile_element/PathElement.h>
#include <openrct2/world/tile_element/SurfaceElement.h>
#include <openrct2/world/tile_element/TileElement.h>
#include <stdexcept>
using namespace OpenRCT2;
namespace H = Ui::Handheld;
namespace W = Ui::Windows;
static void Check(bool b, const char* s)
{
    if (!b)
        throw std::runtime_error(s);
}
#include <fstream>
#include <openrct2-ui/handheld/ConstructionPanel.h>
#include <openrct2-ui/handheld/Dashboard.h>
#include <openrct2-ui/handheld/FlatRides.h>
#include <openrct2-ui/handheld/RideAccess.h>
#include <openrct2/SpriteIds.h>
#include <openrct2/drawing/Drawing.Sprite.h>
#include <openrct2/drawing/G1Element.h>
#include <openrct2/drawing/Palette.h>
#include <openrct2/drawing/X8DrawingEngine.h>
#include <openrct2/management/Research.h>
int main(int argc, char** argv)
{
    try
    {
        if (argc != 4 && argc != 6)
            return 2;
        const int width = argc == 6 ? std::stoi(argv[4]) : 512;
        const int height = argc == 6 ? std::stoi(argv[5]) : 384;
        Check(width > 0 && height > 0, "Invalid window size");
        SDL_setenv("OPENRCT2_HANDHELD", "1", 1);
        gCustomOpenRCT2DataPath = argv[1];
        gCustomUserDataPath = argv[2];
        gOpenRCT2Headless = true;
        gOpenRCT2NoGraphics = true;
        auto env = CreatePlatformEnvironment();
        auto ui = Ui::CreateUiContext(*env);
        auto context = CreateContext(std::move(env), Audio::CreateDummyAudioContext(), std::move(ui));
        Config::SetDefaults();
        Config::Get().general.windowWidth = width;
        Config::Get().general.windowHeight = height;
        Config::Get().general.fullscreenMode = 0;
        Config::Get().general.windowScale = 1;
        Check(context->Initialise(), "Context failed");
        context->GetUiContext().CreateWindow();
        const ScreenSize screen{ context->GetUiContext().GetWidth(), context->GetUiContext().GetHeight() };
        const int scale = H::Layout::AutoScale(width, height);
        Check(screen.width == width / scale && screen.height == height / scale, "Unexpected logical canvas size");
        std::cout << width << 'x' << height << " panel, " << screen.width << 'x' << screen.height << " canvas\n";
        auto* wm = Ui::GetWindowManager();
        wm->CloseAllExceptFlags({});
        WindowCullDead();
        gameStateInitAll(getGameState(), { 64, 64 });
        gLegacyScene = LegacyScene::playing;
        getGameState().park.cash = 10000000;
        getGameState().park.flags.unset(ParkFlag::noMoney, ParkFlag::noMoneyScenario);
        for (int y = 32; y < 2016; y += 32)
            for (int x = 32; x < 2016; x += 32)
                MapGetSurfaceElementAt(CoordsXY{ x, y })->setOwnership({ OwnershipFlag::landOwned });
        auto& objects = context->GetObjectManager();
        Check(objects.LoadObject("rct2.footpath_surface.tarmac"), "Path missing");
        Check(objects.LoadObject("rct2.footpath_surface.queue_blue"), "Queue missing");
        const auto cash = getGameState().park.cash;
        std::filesystem::create_directories(argv[3]);
        gOpenRCT2NoGraphics = false;
        GfxLoadG2PalettesFontsTracks();
        Drawing::initColourMaps();
        FontSpriteInitialiseCharacters();
        auto capture = [&](const char* name, bool placement = false) {
            Drawing::X8DrawingEngine engine(context->GetUiContext());
            engine.Resize(screen.width, screen.height);
            engine.BeginDraw();
            auto& rt = *engine.getRT();
            GfxClear(rt, Drawing::PaletteIndex::pi10);
            if (placement)
            {
                H::Paths::DrawOverlay(rt, H::Concept::kOrigin);
                H::DrawPathFooter(
                    rt, screen, "", H::Paths::IsRemoving(), H::Paths::IsConnected(), H::Paths::IsAdjusting(),
                    H::Paths::ConnectedReady());
            }
            else
            {
                auto* window = wm->FindByClass(WindowClass::handheldMenu);
                Check(window, "Capture window missing");
                window->onDraw(rt);
            }
            Drawing::GamePalette palette{};
            const auto* pal = GfxGetG1Palette(SPR_GAME_DEFAULT_PALETTE);
            for (int i = 0; i < pal->numColours; i++)
                palette[pal->startIndex + i] = { pal->palette[i].blue, pal->palette[i].green, pal->palette[i].red, 255 };
            std::ofstream out(std::filesystem::path(argv[3]) / name, std::ios::binary);
            out << "P6\n" << screen.width << ' ' << screen.height << "\n255\n";
            for (int y = 0; y < screen.height; y++)
                for (int x = 0; x < screen.width; x++)
                {
                    const auto c = palette[static_cast<unsigned>(rt.bits[y * rt.LineStride() + x])];
                    const char rgb[] = { static_cast<char>(c.red), static_cast<char>(c.green), static_cast<char>(c.blue) };
                    out.write(rgb, 3);
                }
            engine.EndDraw();
        };

        H::Paths::Begin(false);
        capture("path-live.ppm", true);
        W::HandheldPathOptionsOpen();
        W::HandheldMenuMove(6);
        capture("path-settings.ppm");
        W::HandheldMenuConfirm();
        Check(W::HandheldObjectPickerIsOpen(), "Surface action missing");
        W::HandheldMenuBack();
        W::HandheldMenuConfirm();
        Check(W::HandheldObjectPickerIsOpen(), "Surface-picker Back lost options focus");
        W::HandheldMenuBack();
        W::HandheldMenuBack();
        Check(H::Paths::IsActive(), "Back ended path tool");
        H::Paths::End();
        H::Paths::BeginConnected(true);
        W::HandheldPathOptionsOpen();
        W::HandheldMenuMove(6);
        capture("connected-settings.ppm");
        W::HandheldMenuConfirm();
        Check(W::HandheldObjectPickerIsOpen(), "Connected surface action missing");
        W::HandheldMenuBack();
        W::HandheldMenuConfirm();
        Check(W::HandheldObjectPickerIsOpen(), "Connected surface-picker Back lost focus");
        W::HandheldMenuBack();
        W::HandheldMenuMove(1);
        W::HandheldMenuConfirm(); // No new section: native undo refuses and retains focus.
        W::HandheldMenuMove(-1);
        W::HandheldMenuConfirm();
        Check(W::HandheldObjectPickerIsOpen(), "Undo error lost surface neighbour/focus");
        W::HandheldMenuBack();
        W::HandheldMenuBack();
        Check(H::Paths::IsConnected() && H::Paths::IsQueue(), "Back lost connected queue");
        H::Paths::End();
        Check(getGameState().park.cash == cash, "Settings spent cash");
        // Build native ride/access fixture, then exercise both point-specific handoffs.
        gOpenRCT2NoGraphics = true;
        for (auto name :
             { "rct2.terrain_surface.grass", "rct2.terrain_edge.rock", "rct2.station.plain", "rct2.footpath_railings.wood" })
            Check(objects.LoadObject(name), "Fixture object missing");
        auto* rideObject = objects.LoadObject("rct2.ride.mgr1");
        Check(rideObject, "Carousel missing");
        const auto index = objects.GetLoadedObjectEntryIndex(rideObject);
        ResearchInsertRideEntry(index, true);
        RideEntrySetInvented(index);
        RideTypeSetInvented(FlatRidePlacement::Type(index));
        Check(H::BeginFlatRide(index), "Ride start failed");
        Check(H::FlatRides::Action(CoordsXY{ 640, 640 }, true).error == GameActions::Status::ok, "Ride placement failed");
        GameActions::ProcessQueue(getGameState());
        const auto target = *H::FlatRides::Target();
        for (int n = 0; n < 2; ++n)
        {
            bool done = false;
            for (int y = 480; y <= 800 && !done; y += 32)
                for (int x = 480; x <= 800 && !done; x += 32)
                    if (H::FlatRides::Action(CoordsXY{ x, y }, false).error == GameActions::Status::ok)
                    {
                        Check(
                            H::FlatRides::Action(CoordsXY{ x, y }, true).error == GameActions::Status::ok,
                            "Access placement failed");
                        GameActions::ProcessQueue(getGameState());
                        done = true;
                    }
            Check(done, "No valid access side");
        }
        H::FlatRides::End();
        Check(W::MainOpen(), "Main viewport missing");
        const auto nativeCash = getGameState().park.cash;
        const auto points = H::RideAccess::Existing(target);
        Check(points.size() == 2, "Native access missing");
        for (int i = 0; i < 2; ++i)
        {
            const auto handoffCash = getGameState().park.cash;
            const auto point = H::RideAccess::Existing(target).at(
                i); // Refresh context after the preceding real landing changed nearby paths.
            W::HandheldRideOpen(target.id);
            Check(W::HandheldRideQuickAccess(), "Modern access shortcut missing");
            W::HandheldMenuMove(i);
            W::HandheldMenuConfirm();
            const auto model = W::HandheldRidesPresentation();
            Check(
                model.rows.size() == 4 && model.rows[2] == (i ? "Connect exit path..." : "Connect entrance queue..."),
                "Point path handoff missing");
            W::HandheldMenuMove(2);
            if (i == 0)
            {
                gOpenRCT2NoGraphics = false;
                capture("access-path-shortcut.ppm");
                gOpenRCT2NoGraphics = true;
            }
            if (i == 0)
            {
                wm->CloseByClass(WindowClass::mainWindow);
                WindowCullDead();
                W::HandheldMenuConfirm();
                Check(W::HandheldRidesIsOpen() && !H::Paths::IsActive(), "Failed path handoff closed inspection");
                Check(
                    W::HandheldRidesPresentation().status.find("Cannot start path") != std::string::npos,
                    "Failed handoff feedback missing");
                Check(W::MainOpen(), "Restore viewport failed");
            }
            W::HandheldMenuConfirm();
            Check(
                H::Paths::IsConnected() && H::Paths::IsQueue() == !i && !W::HandheldRidesIsOpen(),
                "Point handoff lost connected path mode");
            Check(H::RideAccess::Current(point) && getGameState().park.cash == handoffCash, "Path handoff mutated access/cash");
            if (i == 0)
            {
                gOpenRCT2NoGraphics = false;
                capture("connected-queue-overlay.ppm", true);
                gOpenRCT2NoGraphics = true;
                const auto quote = H::Paths::Action(false);
                Check(quote.error == GameActions::Status::ok, "Guided native landing query failed");
                Check(H::Paths::Action(true).error == GameActions::Status::ok, "Guided native landing failed");
                GameActions::ProcessQueue(getGameState());
                InputSetState(InputState::normal);
                H::ControllerInput geometryInput;
                auto geometryPress = [&](H::Action action) {
                    H::Frame f;
                    f.actions = { action, action };
                    H::HandleExplorerInput(geometryInput, f);
                };
                const auto originalGeometry = H::Paths::PlacementName();
                geometryPress(H::Action::contextSecondary);
                H::Frame turn;
                turn.navigateX = 1;
                turn.panX = 8;
                H::HandleExplorerInput(geometryInput, turn);
                Check(
                    H::Paths::IsAdjusting() && H::Paths::PlacementName() != originalGeometry,
                    "Connected direction shortcut failed");
                turn.navigateX = -1;
                turn.panX = -8;
                H::HandleExplorerInput(geometryInput, turn);
                Check(H::Paths::PlacementName() == originalGeometry, "Connected reverse turn failed");
                geometryPress(H::Action::slopeUp);
                Check(H::Paths::PlacementName().find("Rise") != std::string::npos, "Connected trigger slope failed");
                geometryPress(H::Action::confirm);
                Check(H::Paths::PlacementName().find("Flat") != std::string::npos, "Connected Adjust A did not level");
                gOpenRCT2NoGraphics = false;
                capture("connected-adjust.ppm", true);
                gOpenRCT2NoGraphics = true;
                geometryPress(H::Action::back);
                Check(H::Paths::IsActive() && !H::Paths::IsAdjusting(), "Connected Adjust Back ended tool");
                const auto placedCash = getGameState().park.cash;
                Check(placedCash == nativeCash - quote.cost, "Guided native landing accounting changed");
                gOpenRCT2NoGraphics = false;
                capture("connected-endpoint.ppm", true);
                gOpenRCT2NoGraphics = true;
                W::HandheldPathOptionsOpen();
                W::HandheldMenuMove(7);
                W::HandheldMenuConfirm();
                W::HandheldMenuBack();
                W::HandheldMenuMove(-1);
                W::HandheldMenuConfirm();
                Check(W::HandheldObjectPickerIsOpen(), "Undo review Back lost row focus");
                W::HandheldMenuBack();
                W::HandheldMenuBack();
                Check(getGameState().park.cash == placedCash, "Undo Cancel changed accounting");
            }
            H::EndTerrain();
            WindowCullDead();
        }

        const auto navigationCash = getGameState().park.cash;
        H::ControllerInput controller;
        auto press = [&](H::Action action) {
            H::Frame frame;
            frame.actions = { action, action };
            H::HandleExplorerInput(controller, frame);
        };
        auto navigate = [&](int x, int y) {
            H::Frame frame;
            frame.navigateX = x;
            frame.navigateY = y;
            frame.panX = x * 8;
            frame.panY = y * 8;
            H::HandleExplorerInput(controller, frame);
        };
        gOpenRCT2NoGraphics = false;
        W::HandheldDashboardOpen();
        while (W::HandheldDashboardSectionIndex() != 0)
            W::HandheldDashboardMove(0, -1);
        capture("dashboard-overview.ppm");
        W::HandheldDashboardMove(1, 0);
        W::HandheldDashboardSwitchPane();
        W::HandheldDashboardMove(0, 1);
        W::HandheldDashboardMove(1, 0);
        capture("ride-roster.ppm");
        press(H::Action::confirm);
        auto model = W::HandheldRidesPresentation();
        Check(
            model.tabs.size() == 4 && model.selectedTab == 0 && model.rows.front() == "Status" && model.hero,
            "Ride Overview/state switch missing");
        capture("ride-overview.ppm");
        navigate(0, 1);
        Check(
            W::HandheldRidesPresentation().selected == 1
                && W::HandheldRidesPresentation().settingRows == std::vector<size_t>{ 1 },
            "Down from the state switch did not reach the price row");
        navigate(0, 1);
        Check(W::HandheldRidesPresentation().selected == 2, "Down from the price row did not enter the grid");
        navigate(1, 0);
        Check(W::HandheldRidesPresentation().selected == 3, "Grid right moved incorrectly");
        navigate(0, -1);
        Check(W::HandheldRidesPresentation().selected == 1, "Up from the grid top did not return to the controls");
        navigate(0, 1);
        navigate(1, 0);
        const auto overviewFocus = W::HandheldRidesPresentation().selected;
        press(H::Action::nextOption);
        model = W::HandheldRidesPresentation();
        Check(model.selectedTab == 1 && model.rows.front() == "Operations...", "L/R did not enter Operations tab");
        capture("ride-operations.ppm");
        const auto maintenance = FocusRideTabAction("Maintenance...");
        press(H::Action::confirm);
        Check(W::HandheldRidesPresentation().breadcrumb == "Rides / Maintenance", "Maintenance action mapping changed");
        press(H::Action::back);
        model = W::HandheldRidesPresentation();
        Check(model.selectedTab == 1 && model.selected == maintenance, "Maintenance Back lost tab focus");
        press(H::Action::nextOption);
        model = W::HandheldRidesPresentation();
        Check(
            model.selectedTab == 2
                && model.stats == H::RideManagement::Presentation(H::RideManagement::View::statistics, 0, {}, target).stats,
            "Statistics tab changed native data");
        Check(
            H::MeasureManagement({ screen.width - 2 * H::Layout::margin, H::Layout::ContentHeight(screen) }, model).capacity
                >= model.rows.size(),
            "Statistics actions need another page");
        capture("ride-statistics.ppm");
        FocusRideTabAction("Customers...");
        press(H::Action::confirm);
        press(H::Action::back);
        Check(W::HandheldRidesPresentation().selectedTab == 2, "Customer Back did not return directly to Statistics tab");
        press(H::Action::nextOption);
        model = W::HandheldRidesPresentation();
        Check(model.selectedTab == 3, "Settings tab missing");
        capture("ride-settings.ppm");
        const auto demolition = FocusRideTabAction("Demolish...");
        press(H::Action::confirm);
        model = W::HandheldRidesPresentation();
        Check(model.confirmation && model.selected == 1, "Destructive review not Cancel-first");
        capture("confirmation.ppm");
        press(H::Action::nextOption);
        Check(W::HandheldRidesPresentation().selected == 1, "Shoulder escaped review");
        press(H::Action::back);
        model = W::HandheldRidesPresentation();
        Check(
            model.selectedTab == 3 && model.selected == demolition && H::RideManagement::Resolve(target),
            "Cancel lost target/tab/focus");
        press(H::Action::nextOption);
        Check(W::HandheldRidesPresentation().selected == overviewFocus, "Overview tab focus not retained");
        FocusRideTabAction("Admission");
        press(H::Action::confirm);
        Check(W::HandheldRidesPresentation().tabs.size() == 4, "Unavailable admission opened an editor");
        capture("disabled-action.ppm");
        // Admission can also be enabled by native park pricing policy; entering it never buys anything.
        getGameState().park.flags.set(ParkFlag::freeEntry);
        {
            // The price changes in place with Left/Right, one native step per press, no editor or review.
            FocusRideTabAction("Admission");
            const auto before = GetRide(target.id)->price[0];
            navigate(1, 0);
            GameActions::ProcessQueue(getGameState());
            Check(
                GetRide(target.id)->price[0] > before && !W::HandheldRidesPresentation().editing, "In-place price step failed");
            navigate(-1, 0);
            GameActions::ProcessQueue(getGameState());
            Check(GetRide(target.id)->price[0] == before, "In-place price step did not reverse");
        }
        FocusRideTabAction("Admission");
        press(H::Action::confirm);
        Check(W::HandheldRidesPresentation().editing, "Admission action did not open native price editor");
        capture("admission-editor.ppm");
        press(H::Action::back);
        W::HandheldDashboardSwitchPane();
        W::HandheldDashboardMove(1, 0);
        Check(W::HandheldRidesPresentation().selectedTab == 0, "Dashboard section resume lost tab");
        W::HandheldDashboardPause();
        W::HandheldMenuBack();
        WindowCullDead();
        H::Paths::Begin(false);
        InteractionInfo hit{};
        hit.interactionType = ViewportInteractionItem::terrain;
        hit.Loc = { 960, 960 };
        hit.Element = reinterpret_cast<TileElement*>(MapGetSurfaceElementAt(CoordsXY{ 960, 960 }));
        H::Paths::UpdateTargetFromInfo(hit);
        InputSetState(InputState::normal);
        auto before = H::Paths::PlacementName();
        press(H::Action::slopeUp);
        Check(
            H::Paths::PlacementName() != before && !wm->FindByClass(WindowClass::handheldMenu),
            "Free height shortcut opened menu or did nothing");
        capture("path-elevation.ppm", true);
        const auto view = WindowGetMain()->viewport->viewPos;
        press(H::Action::contextSecondary);
        Check(H::Paths::IsAdjusting(), "Y did not enter Adjust mode");
        navigate(1, 0);
        navigate(0, -1);
        Check(WindowGetMain()->viewport->viewPos.x == view.x, "Adjust D-pad leaked horizontal pan");
        capture("path-adjust.ppm", true);
        press(H::Action::confirm);
        Check(getGameState().park.cash == navigationCash, "Adjust A placed a path");
        press(H::Action::openBuild);
        Check(W::HandheldObjectPickerIsOpen(), "Adjust Select lost surface grid");
        capture("path-surface-picker.ppm");
        press(H::Action::back);
        Check(H::Paths::IsAdjusting(), "Surface Back lost Adjust mode");
        press(H::Action::contextPrimary);
        Check(wm->FindByClass(WindowClass::handheldMenu), "Adjust X did not open More");
        press(H::Action::back);
        Check(H::Paths::IsAdjusting(), "More Back lost tool mode");
        press(H::Action::back);
        Check(H::Paths::IsActive() && !H::Paths::IsAdjusting(), "Adjust Back ended tool");
        capture("path-build.ppm", true);
        press(H::Action::back);
        Check(!H::Paths::IsActive(), "Build Back failed to end tool");
        H::Paths::Begin(false);
        Check(!H::Paths::IsAdjusting(), "New tool inherited Adjust mode");
        H::Paths::End();
        Check(getGameState().park.cash == navigationCash, "Navigation/adjustment/cancel changed cash");
        std::cout << "PASS semantic ride tabs / native action mapping / per-tab focus / direct statistics / safe review\n";
        std::cout
            << "PASS direct free path elevation / modal geometry without placement / surface and More return / Build Back\n";
        return 0;
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << '\n';
        return 1;
    }
}
