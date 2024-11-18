# VisualTesterMac

You can feed raw frames obtained from **TrueDepthFusion** into this program, and reconstruct a point cloud from it after the fact. 

# Running VisualTesterMac to process data
1. Build and run TrueDepthFusion on an iOS device
2. In iOS settings, select TrueDepthFusion, then enable *Dump raw frames to Binary PLY*
3. Switch back to the app, then tap SCAN
4. Tap the shutter button to start scanning (this won't actually do the reconstruction, it will just grab raw frames)
5. After it finishes, the iOS share sheet will come up; AirDrop the zip file to your Mac
6. Unzip that zip file
7. Build and run VisualTesterMac
8. Click Open Directory…, then select the folder where you unzipped that scan data
9. Click Assimilate All

<img width="528" alt="image" src="https://github.com/StandardCyborg/StandardCyborgSDK/assets/6288076/6ce7077e-12a8-4157-bbb2-0dacd465c6a5">
