Feedback Loops
Main article: Memory Model#Framebuffer objects
It is possible to bind a texture to an FBO, bind that same texture to a shader, and then try to render with it at the same time.

It is perfectly valid to bind one image from a texture to an FBO and then render with that texture, as long as you prevent yourself from sampling from that image. If you do try to read and write to the same image, you get undefined results. Meaning it may do what you want, the sampler may get old data, the sampler may get half old and half new data, or it may get garbage data. Any of these are possible outcomes.

Do not do this. What you will get is undefined behavior.

https://www.opengl.org/wiki/Framebuffer_Object#Feedback_Loops