Overlays primarily designed for use in osu!

[Showcase video](https://youtu.be/CXavAN9XSjM)

# How to use

The [Releases](https://github.com/ZenoXi/osu-overlays/releases) section contains the latest release, which you can just download, extract and run. After opening the app, you will see the available overlays. Clicking on one will open the settings menu where you can configure overlay parameters. After that, click the "enable" button and enjoy!

**⚠ Many features require data from osu! itself, which is provided by [tosu](https://github.com/tosuapp/tosu).** While tosu is running, enable the integration with a single click inside the Overlay Engine, no extra setup is necessary.

## Available overlays

_Note: overlays marked with ⚡ require tosu to be running._

- ⚡ Real-time leaderboard - a leaderboard showing your real-time rank in the global/country leaderboard while playing
- ⚡ PP counter - the classic counter showing current pp, with simple and clean visuals
- ⚡ UR counter - single number displaying the unstable rate
- ⚡ Combo counter - shows the sizes of each combo between combo breaks
- Cursor trail - a solid cursor trail with configurable colors and size/length
- Enhanced smoke - pressing the 'C' key (or any key you bind) will draw physically simulated smoke
- Smoke trail - your cursor will constantly emit physically simulated smoke

Explanation for the parameters can be found by hovering the labels near the parameters

# Limitations and known problems

- **The app is only available for Windows.** It works on Windows 7/10, but I haven't been able to test it on Windows 11, so it's possible there might be some issues there.
- **The overlay won't be visible if you run osu! in fullscreen mode.** Fullscreen and borderless modes look visually the same, but have some minor performance differences.
- **If the performance is low in general, tweaking the parameters might help.** In smoke simulations, increasing the 'Cell size' parameter will greatly improve your simulation speed at the cost of smoke resolution. If GPU acceleration isn't available and your CPU has a lot of cores, increading thread count might help. Although, even if the simulation is running at a lower fps, it is usually not that noticeable while playing. Fluid mechanics are not simple, and I'm still learning about it and optimizing, so there will be performance improvements in the future.
- **Hardware acceleration is currently available only for Nvidia GPUs supporting CUDA.** Support for other GPUs will be implemented sometime in the future.

# FAQ

- **Are there plans to add Linux/Mac support?** Mac - no, Linux - maybe. This entirely depends if I find it worth to add Linux suport to the UI framework I'm using, since it would be quite a lot of work.
- **Are any other overlays planned?** Many ideas, not so much time to implement them :D. If you have any ideas you can post them in the issues page. If I like the idea or see that many people want/need it, I will implement it.
