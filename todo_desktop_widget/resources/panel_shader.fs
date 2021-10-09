#version 330

uniform vec2 uPanelSize;
uniform float xPosition;
uniform float yPosition;

float roundedBoxSDF(vec2 CenterPosition, vec2 Size, float Radius) 
{
    return length(max(abs(CenterPosition)-Size+Radius,0.0))-Radius;
}
void main()
{
    // The pixel space scale of the rectangle.
    vec2 size = vec2(uPanelSize.x, uPanelSize.y);
    
    // the pixel space location of the rectangle.
    // vec2 location = vec2(xPosition, yPosition + uPanelSize.y);
    vec2 location = vec2(xPosition, yPosition);

    // How soft the edges should be (in pixels). Higher values could be used to simulate a drop shadow.
    float edgeSoftness  = 0.0f;
    
    // The radius of the corners (in pixels).
    float radius = 60.0f;
    
    // Calculate distance to edge.   
    float distance 		= roundedBoxSDF(gl_FragCoord.xy - location - (size/2.0f), size / 2.0f, radius);
    
    // Smooth the result (free antialiasing).
    float smoothedAlpha =  1.0f-smoothstep(0.0f, edgeSoftness * 2.0f,distance);
    
    // Return the resultant shape.
    vec4 quadColor		= mix(vec4(1.0f, 1.0f, 1.0f, 0.0f), vec4(0.7f, 0.7f, 0.7f, 0.95), smoothedAlpha);
    
    // Apply a drop shadow effect.
    float shadowSoftness = 0.0f;
    vec2 shadowOffset 	 = vec2(0.0f, 0.0f);
    float shadowDistance = roundedBoxSDF(gl_FragCoord.xy - location + shadowOffset - (size/2.0f), size / 2.0f, radius);
    float shadowAlpha 	 = 1.0f-smoothstep(-shadowSoftness, shadowSoftness, shadowDistance);
    vec4 shadowColor 	 = vec4(0.2f, 0.2f, 0.2f, 1.0f);
    // gl_FragColor 			 = mix(quadColor, shadowColor, shadowAlpha - smoothedAlpha);
    gl_FragColor 			 = quadColor;
}