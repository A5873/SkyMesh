# SkyMesh Design Specifications

This document defines the visual design standards and guidelines for the SkyMesh project, ensuring consistency across all project materials, documentation, software interfaces, and hardware designs.

## Brand Identity

### Logo Specifications

#### Primary Logo

The SkyMesh logo represents the project's core values of connectivity, resilience, and openness. It consists of two elements:

1. **Logomark**: A stylized orbital mesh network visualization
2. **Wordmark**: "SkyMesh" in the primary typeface

```
┌─────────────────────────────────────────┐
│                                         │
│                  ★ ★                   │
│                 /│\ │\                  │
│                / │ \│ \                 │
│               /  │  ★  \                │
│              ★───┼───★  \              │
│             /    │      \               │
│            /     │       \              │
│           /      │        ★             │
│                                         │
│          S K Y M E S H                  │
│                                         │
└─────────────────────────────────────────┘
```

#### Logo Variations

1. **Full Color**: Primary usage on white/light backgrounds
2. **Reversed**: White logo for use on dark backgrounds
3. **Monochrome**: Single color version for limited color applications
4. **Logomark Only**: For favicons, app icons, and space-constrained applications

#### Clear Space

Maintain a minimum clear space around the logo equal to the height of the "S" in the wordmark.

#### Minimum Size

- **Print**: 1 inch / 25mm width minimum
- **Digital**: 120px width minimum
- **Logomark Only**: 32px width minimum

### Color Palette

#### Primary Colors

| Color | Hex | RGB | Usage |
|-------|-----|-----|-------|
| **Deep Space** | `#0F1C2E` | `15, 28, 46` | Primary background, dark elements |
| **Orbit Blue** | `#1E88E5` | `30, 136, 229` | Primary accent, key UI elements |
| **Satellite Silver** | `#E0E0E0` | `224, 224, 224` | Secondary elements, light backgrounds |

#### Secondary Colors

| Color | Hex | RGB | Usage |
|-------|-----|-----|-------|
| **Solar Gold** | `#FFC107` | `255, 193, 7` | Highlights, warnings, energy indicators |
| **Atmosphere Teal** | `#00BCD4` | `0, 188, 212` | Secondary accents, ground station elements |
| **Terrestrial Green** | `#4CAF50` | `76, 175, 80` | Success states, Earth elements |
| **Signal Red** | `#F44336` | `244, 67, 54` | Error states, critical alerts |

#### Gradient Specifications

| Gradient | Colors | Usage |
|----------|--------|-------|
| **Orbital Gradient** | `#1E88E5` to `#00BCD4` | Hero elements, data visualizations |
| **Space Gradient** | `#0F1C2E` to `#263238` | Background elements, depth effects |
| **Transmission Gradient** | `#1E88E5` to `#FFC107` | Connection visualization, energy indicators |

### Typography

#### Primary Typeface

**Space Grotesk** - A modern sans-serif typeface with technical precision and open character forms

- **Weights Used**: Light (300), Regular (400), Medium (500), Bold (700)
- **Primary Application**: Headings, UI elements, branding

#### Secondary Typeface

**IBM Plex Sans** - A highly legible sans-serif designed for technical documentation

- **Weights Used**: Light (300), Regular (400), Medium (500), SemiBold (600)
- **Primary Application**: Body text, documentation, UI text

#### Monospace Typeface

**IBM Plex Mono** - A complementary monospace font for code and technical specifications

- **Weights Used**: Regular (400), Medium (500)
- **Primary Application**: Code samples, terminal interfaces, technical specifications

#### Font Hierarchy

| Element | Typeface | Weight | Size | Line Height |
|---------|----------|--------|------|-------------|
| H1 | Space Grotesk | Bold | 32px | 40px |
| H2 | Space Grotesk | Bold | 24px | 32px |
| H3 | Space Grotesk | Medium | 20px | 28px |
| H4 | Space Grotesk | Medium | 18px | 24px |
| Body | IBM Plex Sans | Regular | 16px | 24px |
| Small/Caption | IBM Plex Sans | Regular | 14px | 20px |
| Code | IBM Plex Mono | Regular | 14px | 20px |

### Visual Language

#### Iconography

SkyMesh uses a consistent icon system based on simple geometric forms with the following characteristics:

- **Style**: Line icons with 2px stroke width
- **Corner Radius**: 2px
- **Grid**: 24x24px base grid
- **Padding**: Minimum 2px padding from edge

#### Imagery Style

1. **Photography**:
   - High-contrast space imagery
   - Technical hardware closeups with shallow depth of field
   - Documentary-style deployment and community photos

2. **Illustrations**:
   - Isometric technical illustrations for hardware
   - 2D vector network visualizations
   - Data-driven visualizations for network states

#### Design Motifs

1. **Mesh Patterns**: Representing the network topology
2. **Orbital Rings**: Symbolizing satellite paths and connectivity
3. **Node-Link Structures**: Visualizing data routing and connections
4. **Constellation Forms**: Star-like patterns referencing satellite positions

## Technical Diagrams

### System Architecture Diagrams

#### Style Guidelines

- **Layout**: Top-down hierarchical layout with clear layer separation
- **Connectors**: Orthogonal or diagonal lines with consistent angles
- **Components**: Rectangle blocks with rounded corners (4px radius)
- **Color Coding**: Consistent with system components (e.g., satellites use Orbit Blue)
- **Text**: Left-aligned inside shapes, Space Grotesk Medium

#### Standard Elements

```
┌─────────────────┐   Component Block
│                 │
│    Component    │   - 4px corner radius
│                 │   - 12px padding
└─────────────────┘

┌ ─ ─ ─ ─ ─ ─ ─ ─ ┐   Boundary/Container
                      - Dashed line
└ ─ ─ ─ ─ ─ ─ ─ ─ ┘   - 2px dash, 2px gap

        ▲              Directional Flow
        │              - 2px line weight
        │              - Arrow head 6px
        ▼

        ◆              Decision Point
                      
        ●              Endpoint/Terminator
```

#### Layer Representation

For the three-layer SkyMesh architecture:

1. **Orbital Layer**: Top section, satellite icons connected by mesh lines
2. **Protocol Layer**: Middle section, protocol stack blocks
3. **Ground Layer**: Bottom section, ground station icons and end-user devices

### Hardware Schematics

#### PCB Layout Standards

- **Grid**: 1mm base grid
- **Trace Colors**:
  - **Power**: Red
  - **Ground**: Blue
  - **Signal**: Green
  - **RF**: Purple
- **Component Labels**: IBM Plex Mono, 8pt
- **Layer Representation**:
  - Top layer: Solid lines
  - Bottom layer: Dashed lines
  - Internal layers: Dotted lines

#### 3D Model Standards

- **Color Coding**:
  - **PCB**: Green (#0D5F1C)
  - **Connectors**: Black (#1A1A1A)
  - **Modules**: Component-specific
  - **Structure**: Satellite Silver (#E0E0E0)
  - **Solar Panels**: Deep Blue (#0A2463)
- **Materials**: PBR (physically based rendering) materials for realistic visualization
- **Level of Detail**: Three detail levels (high, medium, low) for different visualization contexts

#### Component Representation

Standard symbols for common components consistent with industry standards:

- **SBC**: Detailed outline with key connectors
- **RF Components**: Specific RF symbol with directional indicators
- **Power Systems**: Standardized power iconography
- **Sensors**: Component-specific simplified representation

### Network Topology Diagrams

#### Node Representation

- **Satellite Node**: Star symbol with orbital path indicator
- **Ground Station**: Upward-facing antenna icon
- **Gateway Node**: Hexagonal node with connection indicators
- **End User**: Simple device icon based on type (mobile, IoT, etc.)

#### Connection Types

- **Active High-Bandwidth**: Solid line, 2px (Orbit Blue)
- **Active Low-Bandwidth**: Solid line, 1px (Orbit Blue)
- **Potential Connection**: Dashed line, 1px (Satellite Silver)
- **Failed/Degraded**: Dotted line, 1px (Signal Red)

#### Traffic Indication

- **Line Thickness**: Proportional to bandwidth utilization
- **Animated Elements**: Pulsing dots indicating data flow direction
- **Congestion Indicators**: Color shifts from blue to yellow to red

#### Geographic Projection

- **2D Map Projection**: Modified Robinson projection
- **Coverage Areas**: Semi-transparent color overlays
- **Orbital Paths**: Curved trajectory lines
- **Ground Coverage**: Signal strength heat maps

### Protocol Stack Visualizations

#### Layer Representation

- **Hierarchical Layers**: Stacked horizontal blocks
- **Sub-Protocols**: Nested blocks within layers
- **Headers/Packets**: Segmented rectangles with byte count labels
- **Data Flow**: Vertical arrows between layers

#### Protocol Relationships

- **Dependencies**: Dotted connectors between related protocols
- **Interfaces**: Standardized connection points between layers
- **Optional Components**: Dashed outlines
- **Extensions**: Connected modules with lighter background

#### State Transitions

- **State Diagrams**: Circular nodes with directional transitions
- **Success Paths**: Solid green connectors
- **Error Paths**: Red connectors
- **Decision Points**: Diamond symbols at branch points

## UI/UX Standards

### Interface Guidelines

#### Design Principles

1. **Clarity**: Prioritize clear communication over decoration
2. **Consistency**: Maintain consistent patterns and behaviors
3. **Hierarchy**: Create clear visual hierarchies for information
4. **Efficiency**: Optimize for technical users and operational needs
5. **Accessibility**: Ensure interfaces are usable by all team members

#### Layout System

- **Grid**: 8px base grid with 24px major grid
- **Margins**: Minimum 16px outer margins
- **Gutters**: 16px between major components
- **Containers**: 1200px maximum width for main content
- **Responsive Breakpoints**:
  - Small: 320-639px
  - Medium: 640-1023px
  - Large: 1024-1439px
  - Extra Large: 1440px+

#### Navigation Patterns

- **Primary Navigation**: Horizontal for web, vertical sidebar for applications
- **Secondary Navigation**: Tabs or pills within content areas
- **Breadcrumbs**: For deep hierarchical navigation
- **Search**: Prominent placement for complex applications

#### States & Feedback

- **Loading States**: Branded spinner with progress indication when possible
- **Empty States**: Helpful messaging and next action guidance
- **Error States**: Clear error messaging with remediation actions
- **Success Confirmation**: Minimal non-intrusive confirmations

### Component Library

#### Core Components

1. **Buttons**:
   - Primary: Filled, Orbit Blue
   - Secondary: Outlined, Dark Space
   - Tertiary: Text-only, Orbit Blue
   - Destructive: Filled, Signal Red
   - Sizes: Small (32px), Medium (40px), Large (48px)

2. **Form Elements**:
   - Text Inputs: 40px height, 4px border radius
   - Dropdowns: Consistent with text inputs
   - Checkboxes/Radios: 20px square/circle
   - Toggle Switches: 40px width, 20px height

3. **Cards & Containers**:
   - Standard Card: 8px border radius, light shadow
   - Panel: 0px border radius, border separator
   - Well: Inset container for grouped content
   - Modal: Centered dialog with overlay

4. **Tables & Data**:
   - Data Tables: Horizontal lines between rows
   - List Views: Compact and standard density options
   - Trees: For hierarchical data display
   - Pagination: Page numbers with prev/next

5. **Status Indicators**:
   - Tags: Rounded pill shape for status labels
   - Badges: Circular for counts and notifications
   - Alerts: Color-coded banners for system messages
   - Progress: Linear and circular progress indicators

#### Specialized Components

1. **Monitoring Interfaces**:
   - Status Dashboards: Grid of key metrics
   - System Health Indicators: Color-coded status panels
   - Alert Lists: Prioritized notification displays
   - Timeline Views: Chronological event displays

2. **Control Interfaces**:
   - Command Terminals: Monospace text with syntax highlighting
   - Parameter Controls: Specialized input types for system parameters
   - Deployment Flows: Multi-step workflows with validation
   - Authorization Dialogs: Secure action confirmation

3. **Network Visualizers**:
   - Constellation View: Interactive satellite position display
   - Connection Map: Global connection visualization
   - Traffic Analyzer: Bandwidth and routing visualization
   - Node Inspector: Detailed single-node status view

### Data Visualization Standards

#### Chart Types & Usage

1. **Time-Series Data**:
   - Line Charts: For continuous metrics
   - Area Charts: For cumulative metrics
   - Sparklines: For compact trend indicators

2. **Categorical Data**:
   - Bar Charts: For comparison across categories
   - Radar Charts: For multi-dimensional metrics
   - Heat Maps: For density representation

3. **Relational Data**:
   - Network Graphs: For connectivity visualization
   - Sankey Diagrams: For flow representation
   - Hierarchy Trees: For nested relationships

4. **Spatial Data**:
   - Maps: For geographic distribution
   - Orbital Plots: For satellite position
   - Coverage Overlays: For signal strength

#### Visual Encoding

- **Color**: Consistent with the color palette, use color primarily for categorization not quantitative values
- **Size**: Linear scaling for area representations
- **Position**: Primary encoding for most critical values
- **Shape**: Used sparingly for categorical differentiation
- **Texture**: Only when necessary for accessibility

#### Interactive Features

- **Tooltips**: Appear on hover with detailed metrics
- **Filtering**: Allow hiding/showing data series
- **Zooming**: Time-window and detail level adjustment
- **Brushing**: Select ranges of data for detailed analysis
- **Animation**: Limited to time-series playback and state transitions

#### Accessibility Considerations

- **Color Blindness**: All visualizations must work without color differentiation
- **Screen Readers**: Include alternative text descriptions for charts
- **Keyboard Navigation**: Support for navigating interactive elements
- **High Contrast**: Mode for visibility in various lighting conditions

## Implementation Resources

### Design Assets

All design assets are stored in the following locations:

- **Logo Files**: `/docs/assets/logos/`
- **Icon Library**: `/docs/assets/icons/`
- **Template Files**: `/docs/assets/templates/`
- **Brand Guidelines PDF**: `/docs/assets/SkyMesh_Brand_Guidelines.pdf`

### Development Resources

- **CSS Variables**: `/src/ground-station/frontend/styles/variables.css`
- **Component Library**: `/src/ground-station/frontend/components/`
- **Chart Templates**: `/src/ground-station/frontend/visualizations/`
- **UI Demonstration**: `/src/ground-station/frontend/demo/`

### Design Tools

- **Primary Design Tool**: Figma
- **Diagram Creation**: Draw.io (with SkyM

