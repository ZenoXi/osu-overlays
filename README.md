Overlays primarily designed for use in osu!

[Showcase video](https://youtu.be/CXavAN9XSjM)

# How to use

The [Releases](https://github.com/ZenoXi/osu-overlays/releases) section contains the latest release, which you can just download, extract and run. There is a chance that your antivirus will flag the app as a virus, as seen in the [VirusTotal scan](https://www.virustotal.com/gui/file/f126b6b18a72aa00ce4adfa19bf5c1858b3ac63a939a12f452e124cbf37356f9), although most sources say it's safe. After opening the app, you will see the available overlays. Clicking on one will open the settings menu where you can configure overlay parameters. After that, click the "enable" button and enjoy!

## Available overlays

- Realistic smoke - pressing the 'C' key (or any key you bind) will draw physically simulated smoke
- Cursor trail - your cursor will emit physically simulated smoke

Explanation for the parameters can be found by hovering the labels near the parameters

# Limitations and known problems

- **The app is only available for Windows.** It works on Windows 7/10, but I haven't been able to test it on Windows 11, so it's possible there might be some issues there.
- **The overlay won't be visible if you run osu! in fullscreen mode.** Fullscreen and borderless modes look visually the same, but have some minor performance differences.
- **If your game stutters while playing on osu!stable, your mouse polling rate might is likely the cause.** This issue in particular is very weird, since even if the overlay is doing absolutely nothing, the game will stutter and drop frames when moving a mouse with high polling rate. The fix is to change the polling rate of your mouse to 250hz or 500hz depending on your PC. Tablets seem to already use low enough polling rate to not be a problem. This also isn't an issue on osu!lazer.
- **If the performance is low in general, tweaking the parameters might help.** Increasing the 'Cell size' parameter will greatly improve your simulation speed at the cost of smoke resolution. If your CPU has a lot of cores, increading thread count might help. Although, even if the simulation is running at a lower fps, it is usually not that noticeable while playing. Fluid mechanics are not simple, and I'm still learning about it and optimizing, so there will be performance improvements in the future.
- **Hardware acceleration is currently available only for Nvidia GPUs supporting CUDA.** Support for other GPUs will be implemented sometime in the future.

# FAQ

- **Are there plans to add Linux/Mac support?** Mac - no, Linux - maybe. This entirely depends if I find it worth to add Linux suport to the UI framework I'm using, since it would be quite a lot of work.
- **Will feature X be implemented?** Currently planned features include configurable smoke color, smoke (and maybe fire) intensity based on in-game combo/kiai sections, particles, interaction with clicks. If you have any ideas you can post them in the issues page. If I like the idea or see that many people want/need it, I will implement it.
- **Are any other overlays planned?** Yes, many! I have a combo bar, which shows the individual combo sizes between each combo break, already working and will add that once the smoke overlays are stable. Besides that, there are many ideas, but those will be coming in the future.
- **How long will it take to add new features/fix bugs?** Depending on the bug/feature it can take take between 1 day and a couple of weeks, but since this will be my main project for the time being, expect things to happen quickly.
