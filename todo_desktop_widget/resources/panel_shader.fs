#version 330

uniform vec2 uPanelSize;
uniform float xPosition;
uniform float yPosition;
uniform float cornerRadius;

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
    
    // Calculate distance to edge.   
    float distance = roundedBoxSDF(gl_FragCoord.xy - location - (size / 2.0f), size / 2.0f, cornerRadius);
    
    // Smooth the result (free antialiasing).
    float smoothedAlpha =  1.0f - smoothstep(0.0f, 1.0f, distance);

    gl_FragColor = mix(vec4(1.0f, 1.0f, 1.0f, 0.0f), vec4(0.7f, 0.7f, 0.7f, 0.95), smoothedAlpha);
}