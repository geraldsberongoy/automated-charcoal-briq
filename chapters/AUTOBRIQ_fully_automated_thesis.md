> **Source of truth for AUTOBRIQ.** Text extracted from the thesis .docx on 2026-10-19 (Chapters 1 to 3). Figures, images and equations are not included; the .docx is the original. The earlier semi-automated documents in this folder describe a superseded design and are kept for history only.

## AUTOBRIQ: DEVELOPMENT OF A FULLY AUTOMATED PAPER-CHARCOAL BRIQUETTE  MACHINE

A Thesis Presented to the Faculty of Computer Engineering Polytechnic University of the Philippines Sta.Mesa, Manila

In Partial Fulfilment of the Requirements for the Degree Bachelor of Science in Computer Engineering     by

Gero B. Añonuevo

Gerald S. Berongoy

Marco Gabriel P. Galdiano Jester Carl C. Tiedra

October 2026

## TABLE OF CONTENTS

Page

Title Page I

Table of Contents II

List of Tables III

List of Figures IV

Chapter 1: The Problem and its Setting 1

Introduction 1

Theoretical Framework 4

Conceptual Framework 9

Statement of the Problem 12

Assumptions of the Study 13

Scope and Limitations of the Study 14

Significance of the Study 15

Definition of Terms 18

Chapter 2: Review of Literature and Studies 21

Thematic Organization of Literature and Studies 21

Synthesis of the Reviewed Literature and Studies 54

Chapter 3: Methodology 57

Research Design 57

Flowchart of Research Design 57

Description of Research Instrument Used 60

Mix Design 62

Data Gathering Procedure 63

Material Requirements 65

Software Requirements 74

Statistical Treatment 77

Design Project Flow 80

Block Diagram 81

Process Flowchart 82

System Architecture 91

Prototype Design 93

Specimen Details 96

Multiple Constraints 97

## List of Tables

| Table no. | Table description |  | Page |
|---|---|---|---|
| 1 | Comparative Analysis of Briquette Geometry |  | 42 |
| 2 | Comparative Analysis of Mechanical Durability by Geometry |  | 46 |
| 3 | Optimization of Material Proportions |  | 48 |
| 4 | Comparison of AutoBriq to Existing Briquette Machines |  | 51 |
| 5 | Mix design |  | 62 |
| 6 | Central Processing, Network Routing, and Memory Units |  | 65 |
| 7 | Solar-Charged Portable Power Station Infrastructure |  | 66 |
| 8 | Electromechanical Actuation, Fluidics, and Curing Modules |  | 70 |
| 9 | Closed-Loop Sensory Feedback and Fail-Safe Interlocks |  | 71 |
| 10 | Software Requirements |  | 74 |
| 11 | Environmental and Power Constraints |  | 97 |
| 12 | Material and Output Constraints |  | 98 |
| 13 | Process and Mechanical Constraints |  | 98 |
| 14 | Hardware and Sensor Constraints |  | 99 |
| 15 | Embedded System and Interface Constraints |  | 100 |
| 16 | Software and Network Constraints |  | 101 |
| 17 | Human-Machine Operational Constraints |  | 101 |

## List of Figures

| Figure no. | Figure description |  | Page |
|---|---|---|---|
| 1 | Research Paradigm |  | 10 |
| 2 | Flowchart of Research Design/Process Flowchart |  | 57 |
| 3 | Block Diagram |  | 79 |
| 4 | Process Flow: Initialization, Loading, and Batch Start |  | 82 |
| 5 | Process Flow: Water Dispensing and Mixing State |  | 84 |
| 6 | Process Flow: Mixture Discharge State |  | 85 |
| 7 | Flowchart: Leveling and Compression Stage |  | 86 |
| 8 | Flowchart: Retraction, Unloading, and Batch Completion Stage |  | 87 |
| 9 | System Architecture of the Fully-Automated Paper-Charcoal Briquette Machine |  | 89 |
| 10 | Isometric View of the Prototype Design for the Fully-Automated Paper-Charcoal Briquette Machine |  | 91 |
| 11 | Front View of the Prototype Design for the Fully-Automated Paper-Charcoal Briquette Machine |  | 92 |
| 12 | Rear View of the Prototype Design for the Fully-Automated Paper-Charcoal Briquette Machine |  | 92 |
| 13 | Left and Right Side Views of the Prototype Design for the Fully-Automated Paper-Charcoal Briquette Machine |  | 93 |
| 14 | Top View of the Prototype Design for the Fully-Automated Paper-Charcoal Briquette Machine |  | 93 |
| 15 | Expected Output Briquette Size of the Prototype |  | 94 |

## Chapter 1

THE PROBLEM AND ITS SETTING

### Introduction

Due to rising Liquefied Petroleum Gas (LPG) prices, repurposing paper waste and residual charcoal fines into paper-charcoal briquettes is increasingly relevant. Philippine energy expenditure studies confirm that solid fuels are economic substitutes, leading households to shift demand to alternatives like charcoal as commercial energy costs rise (Dumagan & Abrigo, 2022). Utilizing these materials provides a practical method for localized waste reduction and an alternative solid fuel source, supported by local government initiatives promoting eco-friendly briquettes (Baccay, 2026). However, the production of this reliable heat source is significantly bottlenecked by its conventional reliance on manual labor.

The manual process for producing extended-burn paper-charcoal briquettes is labor-intensive, time-consuming, and physically demanding. The workflow requires repeated human effort during grinding, soaking, mixing, molding, and compression. Conventional production also typically includes a separate sun-drying period before the briquettes can be used as fuel. In this study, the machine cycle is intentionally limited to material processing through compression molding and press retraction; drying is excluded from the automated production cycle so that the machine can complete a batch and become available for the next batch sooner. Final air-drying is treated as a separate post-production conditioning step before combustion testing or use.

This operational bottleneck is confirmed by briquette-making research, which identifies the inadequacy of production technology, rather than slurry chemistry, as the main barrier to accessibility (Mwamlima et al., 2023). The current industry employs two extremes: capital-intensive, fully automated industrial plants that require three-phase grid power, making them unfeasible for off-grid community deployment; or readily available, cheap manual presses. Manual presses, however, fail to provide controlled and repeatable operating parameters, resulting in unpredictable solid fuel density (Cabaces et al., 2023).

To bridge the gap between manual inconsistency and industrial-scale output, purely mechanical presses must be transformed into Computer Engineering-rooted cyber-physical systems. Integrating AUTOBRIQ’s automated control framework reduces reliance on human intuition for moisture ratios and compaction pressure with deterministic, sensor-driven logic. Advancements demonstrate this efficacy: utilizing ADCs, Hall effect sensors, and servo-driven actuators enables automated, precise classification of briquette moisture content via electrical resistance (Pniel, 2026). Governing these stages with IoT-enabled microcontrollers (e.g., ESP architecture), load cells, and HX711 amplifiers ensures exact compaction limits and weight consistency (Thongsan et al., 2024). This computational approach guarantees precision and structural repeatability for localized automation, facilitating real-time telemetry to PWA dashboards and HMIs to improve administrative monitoring and operational efficiency.

This study aims to develop a solar-powered, fully automated paper-charcoal briquetting machine to streamline production. The system utilizes a computerized closed-loop control architecture to autonomously coordinate water dispensing, soaking and mixing in a shared container, gravity discharge into the mold, compression molding, and batch completion. After the operator loads the required raw materials and initiates a batch, the ESP32-based control system governs stage transitions through preset timers, sensor feedback, safety interlocks, and actuator commands, while the on-board touchscreen and wireless dashboard provide monitoring, batch status, and fault notifications. By ending the automated machine cycle after compression molding and press retraction, the design reduces machine-occupied cycle time and allows the next batch to begin sooner. The molded briquettes are subsequently air-dried outside the machine before combustion testing or use.

In the Municipality of Cavinti, Laguna, Barangay Anglas and Barangay Poblacion have been selected as the study areas for assessing household cooking-fuel practices and the potential acceptance of paper-charcoal briquettes. These communities provide a localized setting for examining household fuel preferences, availability, and willingness to consider alternative solid fuels. The information gathered will help establish the relevance of developing an automated briquette production system that utilizes paper waste and residual charcoal fines.

### Theoretical Framework

This study is anchored on four interrelated theoretical strands that explain both the cyber-physical development of AUTOBRIQ and the physical behavior of the paper-charcoal briquettes it produces. The Agile Hardware Development Framework explains how the machine’s mechanical, electronic, and software subsystems are developed and integrated. Process Automation Theory, Lean Manufacturing Principles, and Time and Motion Study justify the transition from manual briquette production to an automated workflow and provide the basis for measuring operational efficiency. Biomass Densification Theory and the Cellulose Fiber Bonding Concept explain how loose paper waste and charcoal fines are transformed into a compact solid fuel. Finally, densification geometry, briquette size, and Combustion Theory justify the selected rectangular-prism form and the performance indicators used to evaluate the final briquette.

Agile Hardware Development Framework

This study is methodologically anchored on the Agile Hardware Development Framework. In the development of a complex cyber-physical system, traditional linear engineering models are highly prone to hardware-software mismatch. To mitigate this risk, AUTOBRIQ was developed through an Agile Hardware approach, where physical mechanics, sensor networks, and software logic are co-designed through continuous, testable iterations rather than a single final assembly.

In this study, the Agile framework provides the methodological justification for the iterative integration of the ESP32 control system. Instead of writing the complete firmware blindly, the Finite State Machine (FSM) was developed and tested in isolated stages alongside the mechanical subsystems, such as the 20 V mixer for wet pulping and the linear actuator rated at 1,500 N for compression. This iterative development process allowed immediate hardware fault isolation. Specifically, it enabled the continuous testing and refinement of the power isolation strategy, ensuring that back-EMF from inductive motor loads was isolated from the logic core before the system advanced to evaluative testing. By adopting Agile as the development lens, the study ensures that AUTOBRIQ is validated for mechanical stability, electrical safety, and logic accuracy prior to continuous operation.

Process Automation, Lean Manufacturing, and Time and Motion Principles

Process Automation Theory supports the transition from traditional manual briquette production to a fully automated machine process. Automation minimizes the need for continuous human intervention in repetitive production tasks by assigning sequential operations to programmed control logic, sensors, and electromechanical components. In AUTOBRIQ, once the operator loads the required raw materials and initiates the batch, the machine automatically performs the production sequence from water dispensing through soaking and mixing, mixture discharge, compression molding, and press retraction. This supports the study’s objective of reducing the dependence on manual effort and operator judgment during the core production process.

Lean Manufacturing Principles further justify the redesign of the manual briquette-making workflow. Traditional briquette production is characterized by repeated human motion, manual mixing, manual pressing, waiting time, and overprocessing, all of which extend cycle time without directly improving product quality. Applying lean principles allows the study to identify these activities as non-value-adding process inefficiencies. By mechanizing the core workflow, AUTOBRIQ is designed to reduce unnecessary motion and improve production continuity. This aligns with Alora and Amaya (2025), who emphasized that reducing motion, transportation, and overprocessing can improve productivity and shorten production cycles.

Time and Motion Study provides the evaluative basis for measuring whether the automated workflow improves production efficiency. Through direct comparison between the traditional manual baseline and the automated machine process, the study can quantify man-hour consumption, throughput time, and production rate. Jesus et al. (2025) support the use of structured time-and-motion measurement in identifying process inefficiencies and validating improvements in multi-operation workstations. Therefore, Process Automation, Lean Manufacturing, and Time and Motion principles collectively justify both the design of AUTOBRIQ and the efficiency metrics used to evaluate it.

Biomass Densification and Cellulose Fiber Bonding

The physical formation of the briquette is explained by Biomass Densification Theory. Waheed et al. (2022) establish that powdered or loose biomass creates handling, transportation, storage, and air-pollution problems that can be addressed by consolidating the material into a higher-density solid of defined form. In this study, loose paper waste and residual charcoal fines are transformed into a usable solid fuel through mechanical compression. Densification therefore produces two important outcomes: increased bulk density and a fixed external geometry.

The Cellulose Fiber Bonding Concept explains why waste paper can function as a natural structural binder. When paper is soaked and broken down into pulp, its cellulose fibers become hydrated and more capable of interlocking with surrounding particles. During mixing and compression, these hydrated cellulose fibers bind with the charcoal fines and help produce a cohesive briquette without the need for synthetic chemical adhesives. Felongco et al. (2025) support this principle by showing that paper-based briquettes can achieve improved compressive strength and burning performance because of the cellulose composition and bonding behavior of paper materials. Thus, cellulose fiber bonding explains the material interaction that allows the compressed paper-charcoal mixture to retain its form after molding.

Densification Geometry, Size, and Combustion Theory

The selected briquette shape is a deliberate engineering decision governed by densification mechanics and Combustion Theory. AUTOBRIQ consolidates the paper-charcoal slurry through uniaxial compaction within a closed die, where a linear actuator advances a flat platen against a fixed mold cavity. Under this mechanism, compaction pressure is related to the actuator’s thrust and the cross-sectional area of the cavity normal to the stroke axis. A solid rectangular-prism geometry was selected because it maintains a constant cross-sectional area throughout the compression stroke. This permits the closed-loop control system to use the calibrated 300 kg S-type load cell to monitor compression force consistently during molding.

The theoretical weight of this relationship is supported by Yusuf et al. (2023), who demonstrated that compaction pressure affects the density, stability, durability, combustion temperature, and combustion rate of charcoal briquettes. Ibitoye et al. (2022) likewise found that compaction pressure, blending ratio, and particle size substantially determine the combustion and physico-mechanical characteristics of composite fuel briquettes. Cabaces et al. (2023) further support the importance of fixed dimensions by showing that machine settings and a defined molding setup help standardize briquette output. In this study, the rectangular-prism mold therefore operationalizes structural repeatability by allowing the same compression condition to be applied across production trials.

The chosen geometry is also consequential after production because the external form of a solid fuel governs the surface area available for oxidation during combustion. Alabi et al. (2023) found that density, size, moisture content, and internal temperature profile collectively influence combustion behavior, mass-loss rate, and burnout time. The solid rectangular prism used in this study is unperforated and has no internal cavity, limiting oxygen exposure primarily to its external faces. This design is theoretically consistent with the objective of producing a briquette with extended burning duration, because it avoids the additional oxidation surface area created by hollow or perforated forms.

The size and shape of the briquette also support practical handling requirements. Flat, mutually parallel faces allow the briquette to rest in a stable orientation, making it easier to stack, store, handle after compression molding, and position on a cooking surface. However, the geometry also presents a trade-off: a prismatic form concentrates mechanical stress at its edges and corners, making these areas more vulnerable to chipping under impact. For this reason, the standardized one-meter drop test is used to quantify structural durability, with the shatter-resistance index serving as the empirical check on whether the compaction pressure achieved by the machine is sufficient to offset this geometric limitation.

Taken together, these theories provide the technical foundation of the study. Agile Hardware Development explains the iterative construction and integration of AUTOBRIQ; Process Automation, Lean Manufacturing, and Time and Motion Study justify the automation of the production workflow and its efficiency evaluation; Biomass Densification and Cellulose Fiber Bonding explain the formation of the briquette; and densification geometry with Combustion Theory explains the selected shape, size, durability testing, and fuel-performance measures. The machine cycle is therefore theoretically grounded as an automated process ending after compression molding and press retraction, while product removal and ambient air-drying remain separate post-production activities.

### Conceptual Framework

The research paradigm illustrated in Figure 1 was utilized to guide the development of AUTOBRIQ, a self-sustaining, solar-powered, and fully automated paper-charcoal briquette machine. The Input-Process-Output (IPO) model was specifically adopted to provide a structured conceptualization of the entire research and development workflow.

The foundational inputs include Raw Materials (physical ingredients), Hardware Components (microcontrollers, sensors, and mechanical modules), and Software Components (digital interfaces and logic frameworks). These flow through the process block's developmental phases to achieve the main objective: "Development of a Fully Automated Paper-Charcoal Briquette Machine." A feedback block establishes a closed-loop system for continuous system refinement.

Figure 1. Research Paradigm

The inputs required for the system's physical and technical operation are divided into three main categories. First, the Raw Materials consist of paper waste, charcoal fines, and water. Second, the Hardware Components required for the system to function effectively include the ESP32 microcontroller, sensors, a touchscreen, servos, actuators, motors, and safety interlock components alongside an autonomous power infrastructure comprising a solar panel, MPPT controller, and battery. Third, the Software Components utilized in the system's development comprise the Graphical User Interface (GUI), the core control logic, and Micro-SD CSV data logging to ensure efficient data tracking and machine operation.

The processes involved in the research consist of a structured methodology used to achieve the study's objectives. It begins with Planning, which involves establishing the problem and solution, defining research parameters, gathering related literature, and creating digital prototypes. This transitions into the Development phase, focusing on designing the Progressive Web App (PWA), programming the Finite State Machine (FSM), integrating autonomous control logic, establishing sensor output processing, and configuring the safety system and emergency handling protocols. Following development is the Testing phase, which employs methods for tracking man-hour consumption, throughput time, production rate, batch yield, and fuel performance to validate system efficiency. Finally, the Implementation phase involves the deployment of the system in communities relying on charcoal.

These inputs and processes produce the final outcome of this study. The feedback loop continuously evaluates the testing metrics and implementation results, routing them back into the input and process stages to guarantee the optimized development of the fully automated paper-charcoal briquette machine.

### Statement of the Problem

This study aims to develop a system that innovates the production of paper-charcoal briquettes, integrating water dispensing, soaking and mixing, mixture discharge, and compression molding, to create a high-efficiency fuel source with minimal human intervention. Specifically, the study seeks to answer the following:

- What are the key considerations in optimizing the utilization of paper waste and charcoal fines as an alternative solid fuel source?

- What are the differences between the automated system and the traditional manual briquette-making process in terms of physical demands, time consumption, and continuous operational oversight?

- What are the design and operational limitations of the proposed automated system, particularly concerning manual material feeding, water-insoluble toxin extraction, and standardized structural boundaries?

- What is the level of effectiveness of the developed automated machine in improving production efficiency and product quality, specifically measured by:

- Total man-hour consumption;

- Throughput time;

- Production rate;

- Batch yield; and

- Fuel performance?

- What is the level of acceptability of the developed automated system, including its Graphical User Interface (GUI), based on the ISO 25010 quality model standard?

### Assumptions of the Study

This developmental research is anchored on the following basic assumptions:

- Operational Assumption: Mechanizing the soaking, mixing, and molding phases of briquette production will significantly increase the production rate and ensure a more uniform briquette density compared to traditional manual methods.

- Material Assumption: The natural cellulose fibers present in soaked waste paper possess sufficient adhesive properties to effectively bind residual charcoal fines without the need for synthetic chemical additives.

- Economic Assumption: There is a sustained local demand for affordable alternative solid fuels due to the escalating and volatile prices of commercial Liquefied Petroleum Gas (LPG).

- System Viability Assumption: A fully automated small-scale production pipeline provides a more practical, cost-effective, and maintainable solution for small-scale solid fuel production compared to fully automated industrial machinery.

### Scope and Limitations of the Study

The study focuses on developing a solar-powered, fully automated briquetting machine that produces rectangular paper-charcoal briquettes optimized for prolonged burn times. The system automates the processing of water, charcoal fines, and prepared waste paper through sequential electromechanical stages: water dispensing, soaking and mixing in a shared container, gravity discharge into a partitioned mold, compression molding, and press retraction.The partitioned mold is intended to produce at least twenty (20) briquettes per batch. Water removal within the machine is entirely mechanical; no thermal drying subsystem is included in the system, and the molded briquettes are air-dried outside the machine as a specimen-preparation step prior to fuel testing. Operations are managed via a dual-interface control framework. An on-machine touchscreen serves as the primary hub for autonomous stage transitions after the operator loads the raw materials and starts the batch, while a localized progressive web app dashboard provides real-time telemetry monitoring and streamlined administrative data management. The system will be powered by a solar panel system for self sustenance.The system is intended to produce at least 20 briquettes per batch. Fuel performance will be evaluated in terms of burning time and heat retention.

The project is bound by specific mechanical, material, and operational constraints. Briquette output is restricted to a single fixed dimension, and the lack of a pre-soaking purification mechanism means insoluble impurities in the waste paper are retained in the final product. Furthermore, the automated workflow proceeds automatically after the operator prepares and loads the raw materials, inserts the mold, and starts the batch, with operating parameters preset before START and no mid-cycle user prompts. After compression and press retraction, the operator manually pulls out the mold containing the formed briquettes; the system therefore automates the production stages themselves rather than the complete material-handling chain. The system further delimits itself to mechanical water removal, so the residual moisture remaining after compression is reduced by ambient air-drying outside the machine and is not a controlled variable of the system.

The geographical scope of the study is limited to Barangay Anglas and Barangay Poblacion in the Municipality of Cavinti, Laguna. These barangays will serve as the designated areas for community-based data gathering and the assessment of potential end-user needs. Findings obtained from participating households and residents will be limited to the selected study areas and shall not be generalized to the entire municipality or province.

### Significance of the Study

The development of the fully automated paper-charcoal briquette machine offers a multifaceted impact that addresses both immediate socio-economic needs and long-term environmental sustainability. The significance of this study is categorized into its contributions to global goals, national and local sectors, and academic advancement.

United Nations’ Sustainable Development Goals (SDGs)

The proposed system directly aligns with several key UN SDGs, specifically Goal 7 (Affordable and Clean Energy), Goal 12 (Responsible Consumption and Production), and Goal 13 (Climate Action). By repurposing discarded paper waste and residual charcoal fines into an efficient solid fuel, the machine promotes a circular economy and reduces dependence on expensive, non-renewable commercial fuels among low-income communities. Furthermore, its integration of a solar panel as a self-sustaining power source directly reinforces Goal 7 by ensuring that productive energy technology remains accessible even in off-grid or electricity-limited settings. This contributes to a broader, sustainable approach to energy consumption and waste management at the community level.

Government and Waste Management Sectors

For the Municipality of Cavinti, Laguna, particularly Barangay Anglas and Barangay Poblacion, the system presents a practical, community-scale solution to existing waste management challenges. By intercepting paper and biomass waste before it reaches landfills, the machine supports the municipality's local waste diversion efforts and reduces the burden on existing disposal infrastructure. Furthermore, it supports governmental environmental initiatives by providing a viable alternative to wood charcoal, thereby indirectly helping to curb the demand that drives illegal tree cutting and timber poaching, which the Department of Environment and Natural Resources (DENR) has directly linked to the unaffordability of commercial cooking fuels nationwide.

Households and Local Communities

The primary intended beneficiaries of this study are households and local communities in Barangay Anglas and Barangay Poblacion, Cavinti, Laguna, particularly those who rely on solid fuels such as charcoal and wood coal as their primary means of cooking due to limited or no access to Liquefied Petroleum Gas (LPG) and electricity. For these community members, commercially purchased charcoal is not merely an alternative but often their only available and affordable heat source. However, the cost and physical effort associated with sourcing charcoal remain persistent burdens. By automating the labor-intensive stages of briquette production and powering the system through a self-sustaining solar panel, the machine empowers local residents and small-scale fuel producers in Cavinti to manufacture high-quality, extended-burn briquettes with reduced physical effort and at minimal recurring cost, providing a reliable, community-adoptable cooking fuel that does not depend on commercial energy infrastructure.

Solid Fuel Manufacturers and Small Businesses

For small-scale fuel producers and local enterprises in Barangay Anglas, Barangay Poblacion, and nearby communities in Cavinti, Laguna, the proposed automated production system may offer a more efficient alternative to labor-intensive manual briquette-making processes. The integration of a Graphical User Interface (GUI) ensures intuitive operation that can be readily adopted by workers without specialized technical training, allowing local businesses to scale their production of uniform, high-density briquettes more effectively and at greater volume.

Future Researchers

The documentation, baseline metrics, automation performance, product consistency, and machine design established in this research will serve as a foundational reference for future academic endeavors. Future researchers can utilize this study to benchmark efficiency, innovate upon the fully automated workflow, or explore further integration of smart technologies in waste-to-energy systems.

### Definition of Terms

For a clearer understanding of this study, the following terms are defined operationally and conceptually as they are used in the context of the research:

Fully Automated Paper-Charcoal Briquette Machine – A machine that performs the production process automatically after the operator starts the cycle.

AUTOBRIQ – The fully automated machine developed to produce paper-charcoal briquettes.

Batch – One production cycle intended to produce at least twenty (20) briquettes.

Burning Duration – The length of time a briquette continues to burn during testing.

Charcoal Fines – Small charcoal particles used as one of the main raw materials.

Embedded Control System – The ESP32-based system that controls the machine’s sensors, motors, actuators, and safety functions.

Finite State Machine (FSM) – The control logic that manages the sequence of machine operations.

Fuel Performance – The burning duration and heat retention of the briquette during testing.

Graphical User Interface (GUI) – The visual interface used to operate and monitor the machine.

Household – Residents of Cavinti, Laguna specifically Barangay Anglas or Barangay Poblacion who rely on solid cooking fuels and serve as target end-users for prototype testing.

Man-Hour Consumption – The amount of human labor time required during production.

Micro-SD Data Logging – The storage of machine and batch data in a Micro-SD card.

Production Rate – The number of briquettes produced within a given period.

Progressive Web App (PWA) – A web-based application used for machine monitoring, batch-status viewing, and administrative data management.

Throughput Time – The time required to complete one machine production cycle.

Water Holding Capacity (WHC) – The ability of charcoal fines to retain water.

Water Retention Value (WRV) – The ability of paper fibers to retain water.

## Chapter 2

REVIEW OF LITERATURE AND STUDIES

### Thematic Organization of Literature and Studies

The increasing cost of commercial household energy sources continues to pressure Filipino households to seek cheaper and more accessible alternatives. As electricity, LPG, and other commercial energy sources become difficult to sustain for low-income and rural users, many households continue to rely on traditional solid fuels such as wood and charcoal (Dumagan & Abrigo, 2022; Pantuhan et al., 2024). However, this dependence creates environmental concerns, especially when rising charcoal demand contributes to illegal tree cutting, timber poaching, and pressure on local forest resources (Meniano, 2026). At the same time, communities continue to face problems in managing paper waste and charcoal fines that could potentially be redirected into useful fuel products (Anonas et al., 2023; Baccay, 2026).

Paper-charcoal briquettes present a practical response to these connected problems because they combine waste reduction with alternative fuel production. Paper waste can serve as a natural binder, while charcoal fines can be densified into a usable solid fuel when properly mixed, molded, compressed, and dried (Chipangura et al., 2024; Lomunyak et al., 2024). However, manual briquette production remains limited by physical labor, inconsistent material preparation, variable molding pressure, long processing time, and the need for continuous human supervision. These limitations justify the development of a fully automated system that integrates grinding, soaking, mixing, and compression molding into a more controlled production process.

This chapter reviews literature and studies related to the development of a fully automated paper-charcoal briquette machine. The discussion coversthe key considerations in utilizing paper waste and charcoal fines, the differences between automated and manual briquette production, the design and operational limitations of the proposed system, the expected effectiveness of the machine in production efficiency and product quality, and the acceptability of the fully automated system and graphical user interface based on ISO/IEC 25010.

Key Considerations in Optimizing the Utilization of Paper Waste and Charcoal Fines

The need for alternative solid fuel is strongly connected to the economic pressure experienced by households that rely on commercial energy. In the Philippines, household energy demand is sensitive to price changes, and solid fuels remain substitutes when electricity, gas, and liquid fuels become financially difficult to sustain (Dumagan & Abrigo, 2022). Castillo-Dida (2024) further links electricity pricing and carbon emissions, showing that energy decisions affect both household cost and environmental outcomes. Pantuhan et al. (2024) add that rural communities in a southern Philippine city often limit the use of electricity and LPG for cooking because of cost, hazards, and access concerns, making conventional wood and charcoal fuels persistent in daily life.

The same pattern appears in international literature on domestic fuel use. Kalli et al. (2022) found that even subsidized LPG programs do not automatically eliminate traditional fuel use because households continue to consider refill cost, income level, and availability. Habib et al. (2023) similarly describe fuel stacking, where households use LPG together with solid biomass fuels instead of completely shifting to cleaner fuel. Laska and Ige (2023) explain that in settings where electricity supply is insufficient or expensive, firewood and charcoal remain dominant domestic fuel sources. These studies support the first SOP by showing that solid fuel dependence is not merely a cultural preference but an economic and access-driven challenge.

Local reports also show that the renewed demand for charcoal creates environmental pressure. Magbanua (2026) reported a decline in LPG use as households shifted toward firewood during periods of high fuel cost, while Meniano (2026) connected the rising demand for wood charcoal with illegal tree cutting and timber poaching. These conditions make paper-charcoal briquettes relevant because they can reduce dependence on freshly cut wood charcoal while using materials that are otherwise treated as waste.

The waste side of the problem is equally important. Anonas et al. (2023) describe the Philippines as facing growing solid waste generation and limited disposal infrastructure, creating a policy push toward waste-to-energy solutions. Burigsay (2025) found that community-level solid waste management enterprises, including charcoal briquetting, can be feasible when supported by capacity building and basic equipment. Baccay (2026) also documented a DENR initiative in Batanes that converted paper waste into eco-friendly charcoal briquettes, showing that paper-based briquetting is already recognized as a practical local waste-reduction strategy.

Paper waste is not automatically an ideal fuel, however, because its performance depends on paper type, fiber quality, binder behavior, and contamination. Felongco et al. (2025) found that cardboard-based paper briquettes had better compressive strength, longer burn duration, and lower porosity than office paper and newspaper briquettes, showing that paper waste must be sorted and selected carefully. Genuino et al. (2022) also found that paper pulp can function as a binder in bio-briquettes, but the best performance depends on the base material and binder combination. These findings show that paper waste utilization involves material variability that must be addressed before efficient production can occur.

Charcoal fines present a related inefficiency. Chipangura et al. (2024) describe charcoal fines as a useful but underutilized residue that requires binding and densification to become a usable solid fuel. Lomunyak et al. (2024) show that loose charcoal dust can be converted into briquettes using paper, starch, or algae binders, but the resulting density, shatter resistance, calorific behavior, and emissions vary by binder proportion. Waheed et al. (2022) further explain that powdered or loose biomass creates handling, transportation, storage, and air-pollution problems unless it is densified into higher-density briquettes. Thus, the main challenge is not only the availability of waste paper and charcoal fines but the inefficient condition of these materials before grinding, soaking, mixing, molding, and drying.

Differences Between Automated and Traditional Manual Briquette Production

Traditional briquette-making is commonly labor-intensive because it requires repeated manual preparation, mixing, molding, pressing, and drying. wondimagegn and Edao (2023) describe conventional briquette preparation steps in which waste paper is used as a binder, water assists mixing, and materials are manually combined before pressing. Lomunyak et al. (2024) produced charcoal-dust briquettes using a manually operated machine and showed that binder proportion affected density, shatter resistance, calorific value, burning rate, and emissions. These studies prove that manual methods can produce usable briquettes, but they also show that manual production depends heavily on human consistency in mixing, proportioning, and pressure application.

The physical demand of manual production is reduced when the process is transferred to machine-assisted equipment. Tesfaye et al. (2023) developed and evaluated a coffee husk biomass briquette machine and measured throughput capacity, densification, resistance to water penetration, shatter resistance, tumbling resistance, calorific value, fixed carbon, and ash content. wondimagegn and Edao (2023) also evaluated a pellet press briquette machine and reported machine capacity and efficiency as key indicators. These studies show that machine-based briquetting changes the work from repetitive physical compression to machine setup, feeding, monitoring, and output handling.

Automated systems also differ from manual production in time consumption. Cabaces et al. (2023) developed a low-cost briquetting system with defined mixing speed, mixing time, compressive force, and briquetting time, producing twenty briquettes under tested operating conditions. This kind of controlled operation contrasts with traditional manual methods because the time for each phase can be measured, repeated, and improved. Escalante and Alcaraz (2025) further showed that auger speed significantly affected machine output capacity, energy consumption, kindling time, briquetting efficiency, density, moisture resistance, shatter resistance, and burn time in an automated screw-type charcoal briquetting machine.

Lean manufacturing literature explains why this mechanical difference matters. Alora and Amaya (2025) found that motion, transportation, and overprocessing extended cycle time and reduced output in a manufacturing process, while lean improvements significantly reduced cycle time and increased productivity. In the proposed briquette machine, unnecessary walking, repeated hand-mixing, manual pressing, and constant visual checking are comparable non-value-added activities. Therefore, automation is not only a mechanical upgrade but also a workflow redesign that reduces wasted movement and waiting time.

Time and Motion Study supports the comparison between manual and automated production. Jesus et al. (2025) developed a data-driven time and motion study tool that reduced analysis effort and processing time by approximately six times while enabling more detailed recording of workstation events. This supports the proposed study's need to compare total man-hour consumption, throughput time, and production rate between the traditional method and the automated system. It also justifies using measured time data instead of relying only on observation or user perception.

The difference in continuous operational oversight is also central. Manual briquetting requires the operator to remain involved in nearly every production step, while full automation shifts the operator's role toward loading materials, monitoring status, and manually pulling out the mold containing the formed briquettes. Escalante and Alcaraz (2025), Cabaces et al. (2023), and Tesfaye et al. (2023) all demonstrate that machine performance can be evaluated through controlled variables rather than pure manual judgment. This directly supports the second SOP because the proposed system is expected to reduce physical demands, shorten production time, and reduce continuous hands-on supervision. Cabaces et al. (2023) developed a system producing twenty cylindrical briquettes under defined mixing and pressing conditions, while Escalante and Alcaraz (2025) used screw extrusion with automated cutting. In comparison, AUTOBRIQ combines soaking and mixing in a shared container and compresses the mixture into a partitioned pull-out mold, with an intended output of at least 20 briquettes per batch. Its actual production rate and labor requirements will be determined through testing.

Design and Operational Limitations of the Proposed Fully Automated System

The proposed system still has design and operational limitations even if it reduces manual labor. One limitation is manual material feeding. Ibitoye et al. (2022) found that compaction pressure, blending ratio, and particle size substantially affected the combustion and physico-mechanical characteristics of composite fuel briquettes, while Yusuf et al. (2023) found that compaction pressure changed density, stability, durability, combustion temperature, and combustion rate in rice husk charcoal briquettes. Cabaces et al. (2023) also established fixed operating conditions for a briquetting system, including mixing speed, compressive force, and briquetting speed, to produce standardized briquettes. These findings indicate that briquette quality is sensitive to material proportion, particle condition, and compaction settings. Therefore, for the proposed system, controlled manual feeding is treated as an operational requirement to help maintain consistent input conditions and reduce the risk of uneven mixing or excessive machine load.

Feedstock variability also limits the grinding and mixing stages. Tesfaye et al. (2023) found that binder ratio affected throughput capacity, densification, water resistance, shatter resistance, ash content, fixed carbon, and calorific value. Lomunyak et al. (2024) similarly found that paper, starch, and algae binders produced different density ranges, shatter resistance, and emission levels. These findings show that the machine cannot guarantee the same output quality unless material ratios and moisture levels are standardized before operation.

Another limitation concerns contaminants in paper waste. Mofokeng et al. (2024) found that recycled paper grades may contain chemical compounds associated with additives, plastics, consumer exposure, sorting, and reprocessing, and that different separation approaches may be needed to reduce contaminant exposure. Mentes et al. (2024) also found that burning paper-based waste with wood can alter emitted particles and increase polycyclic aromatic hydrocarbon concentrations depending on the waste type. These studies support the limitation stated in the SOP, but they should not be interpreted as proof that water soaking removes toxins. Instead, soaking should be treated only as a fiber-softening and pulping step, not as a contaminant-removal process, since paper-based waste may still contain inks, coatings, additives, plastics, or chemical residues that require material selection and separation controls.

The literature on paper fibers also shows that water handling is technically important but not a complete purification method. Sjostrand and Brolinson (2022) found that additives such as polyvinylamine can affect pulp drainage, dewatering, tensile strength, and air permeance, showing that paper fiber behavior depends on chemistry, water retention, and dewatering conditions. Arabi and Dahmardeh Ghalehno (2023) likewise show that drying lignocellulosic biomass involves simultaneous heat and mass transfer and that moisture removal is nonlinear. Therefore, the proposed system must treat soaking as a controlled preparation step rather than as complete detoxification or uniform quality assurance.

The machine also has standardized structural boundaries. Cabaces et al. (2023) produced briquettes with fixed dimensions under a specific molder and compression setup, while Escalante and Alcaraz (2025) showed that auger speed affected output quality in a screw-type briquetting machine. Yusuf et al. (2023) found that compaction pressure affects density, stability, durability, combustion temperature, and combustion rate in rice husk charcoal briquettes. Ibitoye et al. (2022) similarly found that compaction pressure, blending ratio, and particle size substantially affected the combustion and physico-mechanical properties of composite fuel briquettes. These findings justify limiting the proposed machine to a defined briquette size, shape, mold volume, and actuator force range.

Power supply is another boundary of the proposed system. Alfanz et al. (2023) showed that solar power can support smart agricultural equipment, while Nkuriyingoma et al. (2022) demonstrated that photovoltaic systems with battery storage can support small household loads in off-grid conditions. However, these sources also imply that solar-powered operation depends on panel sizing, battery capacity, load demand, and solar availability. Thus, the proposed system may be self-sustaining, but its operating schedule and motor load must remain within the limits of the solar-battery design.

Effectiveness of the Developed Machine in Production Efficiency and Product Quality

Effectiveness must first be measured through labor and time savings. Alora and Amaya (2025) show that cycle time and output can be used to identify production inefficiencies, especially those caused by unnecessary motion, transportation, manual handling, and redundant steps. Jesus et al. (2025) show that time and motion study can be strengthened through data-driven recording of workstation events. These studies support the use of total man-hour consumption and throughput time as effectiveness indicators for the proposed machine.

Production rate is another major measure. Cabaces et al. (2023) evaluated a briquetting system using mixing rate, briquetting rate, percent yield, and energy efficiency. Escalante and Alcaraz (2025) reported that an automated screw-type charcoal briquetting machine reached its highest output capacity at a specific auger speed range, while another speed range produced higher briquetting efficiency and stronger briquettes. Tesfaye et al. (2023) and wondimagegn and Edao (2023) also measured machine capacity in evaluating briquetting equipment. These studies show that production rate should be measured together with quality because the fastest machine setting is not always the best setting for briquette strength or fuel performance. For AUTOBRIQ, batch yield will be evaluated against the intended output of at least 20 briquettes per batch.

Fuel performance depends heavily on density, moisture, and compaction. Yusuf et al. (2023) found that compacting pressure affects the mechanical and combustion characteristics of rice husk charcoal briquettes. Waluyo et al. (2022) found that blending ratio affected stable density, water resistance, compressive strength, and higher heating value in sawdust-cocoa pod briquettes. Ibitoye et al. (2022) found that compaction pressure, particle size, and blending ratio affected calorific value, density, shatter index, and water resistance. These findings support measuring product quality beyond visual appearance.

Binder performance is also a key indicator of output quality. Obi et al. (2022) reviewed biomass briquette binders and emphasized that binders improve cohesive strength and affect mechanical and handling properties. Malovanyy et al. (2023) found that adding a natural binder could reduce required forming pressure, reduce engine power, and increase briquette density. Montiel et al. (2023) found that algal biomass can function as a binder for eco-briquettes, while Lomunyak et al. (2024) found that paper-bound charcoal dust briquettes achieved high shatter resistance. These studies support using soaked paper pulp as a natural binder while still testing whether the final product is durable.

Burning performance must also be tested through combustion indicators. Felongco et al. (2025) found that cardboard-based paper briquettes produced higher compressive strength, longer burn duration, and lower porosity than other paper types. Dumrique et al. (2024) tested agricultural residue briquettes through water-boiling, temperature, burning rate, and acceptability measures, showing that fuel performance can be evaluated through practical cooking-related indicators. Waheed et al. (2022) explain that densification improves handling and energy density while addressing the limitations of loose biomass. These sources support evaluating the briquettes’ fuel performance through burning time and heat retention.

Environmental and health-related fuel performance also matters. Qi et al. (2022) found that improved biomass combustion technology can reduce pollutant emissions, including large reductions in PM2.5 under improved stove conditions. Glalah et al. (2024) found that binder type affected CO and PM2.5 emissions from biochar briquettes and reported several briquette formulations within safe air-quality thresholds for domestic use. Mentes et al. (2024), however, warn that burning mixed waste materials can increase harmful PAH concentrations. These findings show why the developed machine must produce consistent molded briquettes from selected feedstock that can undergo proper post-production air-drying rather than uncontrolled mixed waste.

Finally, effectiveness includes the machine's ability to operate in the intended setting. Alfanz et al. (2023) demonstrate the feasibility of applying solar power to smart agricultural systems, and Nkuriyingoma et al. (2022) show that photovoltaic systems with battery storage can provide reliable off-grid power for small household applications. For the proposed machine, this supports evaluating whether the solar-powered control system can sustain water dispensing, soaking and mixing, mixture discharge, compression molding, and press retraction operations without depending entirely on grid electricity.

Acceptability of the Fully Automated System and GUI Based on ISO 25010

Acceptability should be understood as both technical acceptability and user acceptability. Burigsay (2025) found that community-based solid waste management enterprises, including charcoal briquetting, are viable when they are supported by equipment, capacity building, and stakeholder participation. Mwamlima et al. (2023) also note that briquette-making technologies can support sustainable cooking energy but may be limited by complexity and cost. These findings suggest that the proposed machine must not only work technically; it must also be understandable, manageable, and acceptable to the intended users.

ISO/IEC 25010:2023 provides the appropriate framework for evaluating the fully automated system and GUI because it defines a product quality model for ICT and software products and supports requirements specification, testing objectives, quality control criteria, acceptance criteria, and product-quality measures (International Organization for Standardization & International Electrotechnical Commission [ISO/IEC], 2023). Since the proposed machine includes an embedded controller and graphical user interface, ISO/IEC 25010 is suitable for evaluating whether the interface supports the user's required tasks, gives understandable feedback, performs reliably, and assists safe operation.

Recent software-quality literature supports applying ISO/IEC 25010 to user-facing interfaces, although the context of each study must be considered. Carrión-León et al. (2025) evaluated the interaction capability of a serious-game graphical interface for children with autism spectrum disorder using ISO/IEC 25010:2023, focusing on operability and inclusiveness through interface interaction data such as clicks, visual focus, level selection, feedback components, time, moves, and scores. While this is not an industrial machine study, it is useful as an example of how ISO/IEC 25010 can be operationalized into observable GUI criteria. In the proposed machine, the transferable point is the evaluation approach: users must be able to monitor automated processes such as water dispensing, soaking and mixing, mixture discharge, compression molding, press retraction, and overall system status without confusion.

Functional suitability is important because the GUI must cover the actual machine operations required by the study. The interface should provide controls and indicators for process activation, cycle monitoring, motor status, and completion alerts. If any of these functions are missing or unclear, the user may still need continuous manual oversight, weakening the purpose of automation. Thus, functional suitability directly connects to the SOP's concern for minimal human intervention.

Usability or interaction capability is equally important because the system may be used by students, operators, or community users who are not specialists in embedded systems. The GUI should use clear labels, visible process states, and error alerts to reduce operator confusion. Reliability and performance efficiency must also be evaluated because delays, freezes, unreadable values, or inconsistent button responses can interrupt production and reduce trust in the machine.

Safety and maintainability are also part of acceptability. Since the machine includes moving parts, heat, moisture, and solar-powered electrical components, the GUI must support safe operation through warnings, status feedback, and predictable controls. The solar-power studies of Alfanz et al. (2023) and Nkuriyingoma et al. (2022) support the feasibility of localized power systems, but ISO/IEC 25010 helps determine whether the user-facing control system remains acceptable during actual operation. Therefore, the acceptability evaluation should include both user ratings and observed system behavior based on selected ISO/IEC 25010 quality characteristics.

Production Process and System Workflow of Paper-Charcoal Briquette Making

The production process of paper-charcoal briquette making involves paper preparation, mixing, compression molding, and external air-drying. These stages explain how the overall production workflow converts waste paper and charcoal fines into a usable briquette product.In AUTOBRIQ, paper preparation is completed before loading, while soaking and mixing occur in a shared container. Mold removal is performed manually after compression and press retraction, and air-drying occurs outside the machine.

Paper waste preparation serves as the initial stage of the production workflow. Genuino et al. (2022) show that paper pulp can function as a binder in bio-briquettes, while Felongco et al. (2025) found that different paper types produce different briquette outcomes in terms of compressive strength, porosity, and burning duration. Lomunyak et al. (2024) also demonstrated that paper can be used as a binder for charcoal-dust briquettes, but binder proportion affects density, shatter resistance, calorific value, burning rate, and emissions. These findings support soaking and pulping as a process step, but only for fiber softening and binder preparation, not toxin extraction.

Mofokeng et al. (2024) found that recycled paper grades may contain chemical compounds associated with additives, plastics, consumer exposure, sorting, and reprocessing, while Mentes et al. (2024) showed that burning paper-based waste with wood can alter emitted particles and increase harmful compound concentrations depending on waste type. Therefore, soaking is treated only as a fiber-softening and pulping step, while paper sorting and contaminant avoidance remain design and operational limitations.

Soaking and mixing prepared paper with charcoal fines in a shared container form the next stage of the production workflow. Chipangura et al. (2024) describe charcoal fines as an underutilized residue that requires binding and densification before it can become a usable solid fuel. Malovanyy et al. (2023) found that natural binders can improve briquette formation by reducing forming pressure and increasing briquette density, while Obi et al. (2022) explain that binders improve cohesive strength and handling properties in biomass briquettes. These studies support the system’s mixing stage because it is intended to promote contact between wet paper fibers and charcoal particles before molding and compression.

Mechanical molding and compression form the next stage of the production workflow. Cabaces et al. (2023) developed a low-cost briquetting system with defined mixing speed, compressive force, briquetting speed, and fixed briquette dimensions, showing that machine settings can standardize output geometry. Yusuf et al. (2023) found that compaction pressure affects briquette density, stability, durability, combustion temperature, and combustion rate, while Ibitoye et al. (2022) found that compaction pressure, blending ratio, and particle size affect combustion and physico-mechanical properties. These findings justify the molding and compression stage because it directly connects the mechanical design to the expected product quality.

Theoretical Framework: Agile Hardware Development for Cyber-Physical System Integration

This study is guided by an Agile hardware-software development framework. Drutchas and Eppinger (2022) explained that Agile began in software development but has increasingly been applied to combined hardware and software projects, where teams must adjust Agile practices because hardware work involves physical constraints, sprint-cadence limitations, and backlog-creation challenges. This makes Agile hardware development suitable as a guiding framework for the proposed paper-charcoal briquette machine because the system combines mechanical parts, a mixer , a linear actuator, power electronics, embedded firmware, sensor feedback, and a graphical user interface.

The key value of Agile hardware development is that it treats system design as a sequence of testable iterations rather than one final assembly. Drutchas and Eppinger (2022) identify combined hardware-software development as a setting where coordination and integration points must be managed carefully. For this study, that means the mixing subsystem, trapdoor actuator, compression actuator, power supply, ESP32 controller, and GUI should be developed and checked in stages. Each stage may be treated as an iterative subsystem validation step where mechanical behavior, electrical stability, and control logic are checked before the next subsystem is integrated.

Fagcang et al. (2022) support this staged approach through their review of component-in-the-loop and hardware-in-the-loop methods, which combine physical subsystems with real-time virtual or control environments to accelerate development and integration. This principle is applicable to the proposed machine because each physical subsystem can be tested under controlled conditions before full-system deployment. In the proposed machine, this supports isolating and testing the 20 V mixer and relay-controlled linear actuators for trapdoor movement and compression before evaluating continuous operation.

The embedded software side of the framework is supported by Finite State Machine logic. Barkalov et al. (2024) describe finite state machines as structured digital systems whose behavior depends on defined states, transitions, inputs, outputs, transition functions, and output functions.In the proposed system, this FSM logic can be applied to firmware organization by separating operations into distinct states such as idle and mold checking, pre-start seal verification, measured water inflow, submerged soaking and mixing, gravity discharge and trapdoor resealing, force-controlled compression, press retraction, and batch completion with manual mold extraction. Using an FSM-based firmware structure supports modular testing because each machine state can be checked against its expected inputs, outputs, and transitions before full system integration.

The Agile hardware framework also supports early electrical-risk isolation. Figueiroa and Torres (2022), in a microcontroller-based smart greenhouse design, described sensors and actuators controlled through electrical subsystems and discussed the inductive flyback effect in a fan circuit, using a flyback diode as the protection solution. A comparable concern applies to the proposed briquette machine’s inductive loads, such as motors and actuator circuits, where protection circuits should be validated before continuous machine trials. Therefore,the logic controller, switching relays, and actuator circuits should be validated before continuous machine trials.

Applied to this study, Agile hardware development provides the development rationale for co-designing physical mechanics, sensor networks, embedded firmware, GUI controls, and power protection. Rather than delaying firmware and control testing until full assembly, the proposed machine should be developed through repeated subsystem validation. This includes testing the mixer, trapdoor actuator, compression actuator, sensor feedback, GUI process commands, and fault responses in isolated cycles. The framework therefore shifts the development process from a rigid assembly task to a responsive engineering process that can identify faults earlier.

Through this framework, the proposed system is treated not only as a finished machine but as a cyber-physical architecture whose mechanical stability, electrical safety, and logic accuracy should be checked through progressive integration. The Agile hardware development framework supports the system’s aim of minimal human intervention because automation is reliable only when the controller, actuators, sensors, and interface operate consistently together. This framework provides the basis for developing, testing, and evaluating the proposed machine as an integrated cyber-physical system.

Comparative Analysis of Briquette Geometry

| Study (Author, Year) | Geometry Tested | Burn Duration | Peak Temperature (°C) | Combustion Rate | Key Finding |
|---|---|---|---|---|---|
| Kah’fi et al. (2025) | Cylindrical, spherical, cubic, rectangular block | Cylinder: 75 min; sphere: 70 min; cube: 75 min; rectangular block: 65 min | Cylinder: 56.2; sphere: 61.8; cube: 58.1; rectangular block: approximately 57.8 | Cylinder: 0.867 g/min; sphere: 0.928 g/min; cube: 0.906 g/min; rectangular block: 1.015 g/min | Rectangular briquettes burned for the shortest duration and had the fastest fuel-consumption rate, while cylindrical and cubic briquettes sustained longer burning. |
| Usaka, Edeoja, and Ibrahim (2023) | Solid cylindrical and hollow cylindrical bio-coal briquettes | Total burnout duration not reported; water-boiling time: 4.25-10.97 min, with hollow briquettes lower than solid briquettes | Not reported | 1.08-5.85 g/min, with hollow briquettes higher than solid briquettes | The central hole increased porosity and oxygen supply, reduced ignition time from 135-195 s in solid briquettes to 103-175 s in hollow briquettes, and increased burning rate. |
| Dragusanu, Lunguleasa, and Spirchez (2022) | Solid wheat-straw briquettes without hollow and hollow wheat-straw briquettes with 26 mm inner opening | Not reported | Not reported | Not reported | Hollow briquettes retained fuel-grade calorific performance, with 1178 kg/m3 density and 20.856 MJ/m3 calorific density, compared with solid briquettes at 1237 kg/m3 and 21.882 MJ/m3 for T1 and 1169 kg/m3 and 20.679 MJ/m3 for T2. |
| Agustinus et al. (2025) | Honeycomb charcoal briquettes with 4, 8, and 12 holes | 4 holes: 35 min water-boiling duration; 8 holes: 25 min; 12 holes: 35 min | 4 holes: 589; 8 holes: Not reported; 12 holes: 427 | 8 holes: 2.8 g/s; 4-hole and 12-hole variants: 2.86 g/s | The 8-hole geometry balanced oxygen supply and heat retention, while 4 holes restricted aeration and 12 holes caused excessive airflow and thermal dissipation. |
| Gebremariyam et al. (2025) | Oval/dumbbell-shaped two-hole perforated briquettes | Burning time values: 36, 40, 43, 45, 49, and 46 min for selected samples; overall range also stated as 29.10-43.41 min | Not reported | Not numerically reported | Side perforations regulated oxygen flow, improved ignition efficiency, and produced ignition times of 1.39-2 min with low CO emissions of 1.8-14.5 ppm. |

Table 1. Comparative Analysis of Briquette Geometry

Influence of Briquette Geometry on Combustion Duration

Briquette geometry altered fuel-consumption behavior under controlled material composition, as cylindrical and cubic rice-husk briquettes burned for 75 min, spherical briquettes burned for 70 min, and rectangular briquettes burned for only 65 min (Kah’fi et al., 2025). The corresponding combustion rates were 0.867 g/min for cylindrical briquettes, 0.906 g/min for cubic briquettes, 0.928 g/min for spherical briquettes, and 1.015 g/min for rectangular briquettes, indicating that the rectangular block consumed fuel fastest while cylindrical and cubic geometries sustained longer fire (Kah’fi et al., 2025).

The introduction of a central hole increased internal oxygen penetration because hollow Prosopis bio-coal briquettes were molded with a central channel intended to increase porosity and oxygen supply (Usaka et al., 2023). Compared with solid briquettes, hollow briquettes recorded shorter ignition time of 103-175 s instead of 135-195 s, higher burning rate within the reported 1.08-5.85 g/min range, and lower water-boiling time within the 4.25-10.97 min range (Usaka et al., 2023).

The airflow benefit of perforation depended on the number of holes, since 4-hole honeycomb briquettes required 35 min to boil 1 L of water due to restricted aeration, 12-hole briquettes also required 35 min due to excessive airflow and thermal dissipation, and 8-hole briquettes required only 25 min because oxygen supply and heat transfer were balanced (Agustinus et al., 2025). The same comparison showed peak combustion temperatures of 589°C for 4-hole briquettes and 427°C for 12-hole briquettes, while the 8-hole design maintained more uniform thermal output and stronger thermal efficiency (Agustinus et al., 2025).

Hollow geometry can preserve fuel-grade heat content when density remains sufficient, as hollow wheat-straw briquettes reached 1178 kg/m3 density and 20.856 MJ/m3 calorific density compared with solid wheat-straw briquettes at 1237 kg/m3 and 21.882 MJ/m3 for T1 and 1169 kg/m3 and 20.679 MJ/m3 for T2 (Dragusanu et al., 2022). Perforated oval briquettes further demonstrated the combustion role of internal airflow, with ignition times of 1.39-2 min, burning durations reported at 29.10-43.41 min, and side openings associated with regulated oxygen flow, minimal smoke, and improved burning efficiency (Gebremariyam et al., 2025).

For sustained briquette combustion, the engineering advantage is not the maximum surface opening but the controlled balance between density, oxygen access, and heat retention (Usaka et al., 2023; Agustinus et al., 2025). Solid rectangular blocks showed faster fuel depletion than cylindrical and cubic forms, while hollow and honeycomb geometries improved ignition and heat transfer but increased consumption or heat loss when airflow became excessive (Kah’fi et al., 2025; Usaka et al., 2023; Agustinus et al., 2025).

Comparative Analysis of Mechanical Durability by Geometry

| Study (Author, Year) | Geometry Tested | Compressive Strength / Shatter Index | Key Durability Finding |
|---|---|---|---|
| Akogun and Waheed (2022) | Cylindrical and square briquettes produced from cornhusk, sawdust, and cassava peel | Highest recorded cylindrical cassava-peel briquette: 0.75 N/mm2 compressive strength and 98.35% shatter index; cylindrical CH shatter index: 95.6-97.2%; cylindrical SD: 91.1-95.8%; cylindrical CP: 96.7-98.4%; square SD: 90.66-95.11%; square CP: 96.44-97.86% | Cylindrical briquettes exhibited higher density, shatter index, compressive strength, and overall physical stability than square briquettes; square briquettes showed reduced cohesive strength at the four edges during compression and ejection. |
| Usaka, Edeoja, and Ibrahim (2023) | Solid cylindrical and hollow cylindrical bio-coal briquettes | Solid: 4.32-5.58 N/mm2 compressive strength and 95.05-98.23% impact resistance; hollow: 3.67-4.74 N/mm2 compressive strength and 87.73-90.67% impact resistance | Solid briquettes had higher compressive strength and impact resistance than hollow briquettes, while hollow briquettes showed lower physical and mechanical properties due to the internal hole. |
| Dragusanu, Lunguleasa, and Spirchez (2022) | Solid cylindrical wheat-straw briquettes without hollows and cylindrical wheat-straw briquettes with hollows | Solid T1: 2.10 N/mm2 compressive strength, 598 kg mean force, 23 maximum stack rows; solid T2: 1.16 N/mm2, 501 kg, 19 rows; hollow T3: 1.14-1.15 N/mm2, 395 kg, 17 rows; splitting strength: T1 = 0.2208 N/mm2, T2 = 0.1712 N/mm2, T3 = 0.3902 N/mm2 | Hollow briquettes had lower compressive strength and stacking capacity than solid briquettes because the internal hollow reduced the load-bearing cross-sectional area. |

Table 2. Comparative Analysis of Mechanical Durability by Geometry

Influence of Geometry on Mechanical Durability

Briquette geometry directly affects mechanical durability because the applied compaction load, ejection force, and later handling stresses are distributed differently across rounded, cornered, solid, and hollow forms (Akogun & Waheed, 2022; Dragusanu et al., 2022; Usaka et al., 2023). Cylindrical briquettes produced higher density, shatter index, compressive strength, and physical stability than square briquettes, while square briquettes showed a loss of cohesive strength at the four edges during compression and ejection, which reduced density and stability (Akogun & Waheed, 2022). This indicates that cornered geometries introduce structural weak points at the edges, whereas rounded cylindrical geometries distribute stress more uniformly and reduce the tendency for edge damage during handling (Akogun & Waheed, 2022). Internal void space produced a similar weakening effect, as solid cylindrical bio-coal briquettes reached 4.32-5.58 N/mm2 compressive strength and 95.05-98.23% impact resistance, while hollow cylindrical briquettes reached only 3.67-4.74 N/mm2 compressive strength and 87.73-90.67% impact resistance (Usaka et al., 2023). Wheat-straw briquettes showed the same pattern, since solid T1 briquettes reached 2.10 N/mm2 compressive strength and supported 23 stack rows, whereas hollow T3 briquettes reached only 1.14-1.15 N/mm2 and supported 17 stack rows due to the reduced resistant cross-sectional area created by the inner hollow (Dragusanu et al., 2022). Rounded solid geometries therefore provide stronger resistance to compression, impact, stacking, and transport-related damage than cornered or internally hollowed forms (Akogun & Waheed, 2022; Dragusanu et al., 2022; Usaka et al., 2023).

Optimization of Material Proportions

| Study (Author, Year) | Material Blend | Reported Material Ratio | Fuel Performance Highlights |
|---|---|---|---|
| Kebede et al. (2022) | Biomass residue:waste paper pulp | 3:1 | Paper-pulp briquettes produced 41.50% fixed carbon, 9.33% ash, 4406.97 cal/g calorific value, and 0.03% sulfur. |
| Lomunyak et al. (2024) | Acacia charcoal dust:paper pulp | 75:25 | The 25% paper-binder formulation produced the highest charcoal-paper calorific value at 23.15 MJ/kg. |
| Lomunyak et al. (2024) | Acacia charcoal dust:paper pulp | 70:30 | The 30% paper-binder formulation produced the shortest ignition time at 0.416 min and the lowest paper-binder burning rate at 0.113 g/min. |
| Lomunyak et al. (2024) | Acacia charcoal dust:starch | 74.9:25.1 | Response surface optimization predicted 23.41 MJ/kg calorific value, 0.862 min ignition time, and 0.0552 g/min burning rate. |
| Kah’fi et al. (2025) | Rice husk charcoal:tapioca starch | 80:20 | The 80:20 formulation showed geometry-dependent combustion rates from 0.867 to 1.015 g/min. |
| Glalah et al. (2024) | Biochar:binder | 6:1 | Sawdust/starch briquettes produced 24.023 MJ/kg calorific value, 0.0483 kg/L specific fuel consumption, and 0.0005 kg/min burning rate. |
| Oladosu et al. (2023) | Flamboyant pod:corn cob:cassava starch | 30:52:18 | The optimized briquette recorded 185 s ignition time, 0.032 g/s fuel burning rate, 196 ppm CO, 0.09 ppm PM2.5, and 0.33 ppm PM10. |
| Murakami and Sato (2024) | Water hyacinth biochar:molasses:organic matter | 6:4:0 | WHB400 with molasses and no organic matter produced the highest heating value and greatest compressive strength among the tested water hyacinth briquettes. |
| Suwankamnoed et al. (2025) | Oil palm frond biochar:red clay:starch | 60:35:5 | The T4 formulation produced 5650 cal/g calorific value, 80 min burning duration, 1.33 ppm CO, 556.30 ppm CO₂, and 0.004 mg/m³ TVOC. |
| Sekyere et al. (2025) | Biomass:cassava starch binder | 50:50 | Coconut husk briquettes had the lowest burning rate at 197 g/h, while corn cob briquettes had the highest HHV at 25.22 MJ/kg. |

Table 3. Optimization of Material Proportions

Optimization of Material Proportions

A 3:1 biomass residue-to-waste-paper-pulp ratio produced higher fuel-quality indicators than clay-bound briquettes, establishing a defensible material proportion for paper-based briquette formulation (Kebede et al., 2022). Paper-pulp-bound briquettes recorded 41.50% fixed carbon, 4406.97 cal/g calorific value, 9.33% ash, and 0.03% sulfur, while clay-bound briquettes recorded 38.49% fixed carbon, 3965.33 cal/g calorific value, 14.86% ash, and 0.06% sulfur (Kebede et al., 2022). The higher fixed carbon and calorific value of paper-pulp briquettes indicate improved solid fuel energy potential at the 75:25 biomass-to-paper-pulp proportion (Kebede et al., 2022).

For charcoal fines, binder proportion significantly affected calorific value, ignition time, and burning rate across the 10–30% binder range (Lomunyak et al., 2024). Charcoal-paper briquettes at 25% paper binder produced the highest charcoal-paper calorific value of 23.15 MJ/kg, while 30% paper binder produced the shortest ignition time of 0.416 min and the lowest charcoal-paper burning rate of 0.113 g/min (Lomunyak et al., 2024). The 75:25 charcoal-fines-to-paper-pulp ratio therefore provides the strongest balance for calorific value, while the 70:30 ratio favors faster ignition and lower burning rate (Lomunyak et al., 2024). These findings guide the formulation trials used to select AUTOBRIQ’s final paper-to-charcoal ratio.

Cellulose as an Independent Binding Agent

Waste paper pulp functions as an independent binder when paper is soaked, pulped, homogenized with charcoal or biomass particles, compacted, and dried (Kebede et al., 2022; Lomunyak et al., 2024). Paper pulp prepared by soaking waste paper in cold water and pounding it into pulp produced briquettes with higher fixed carbon, higher calorific value, lower ash, and lower sulfur than clay-bound briquettes (Kebede et al., 2022). Paper pulp prepared by soaking shredded waste paper for three days produced charcoal-paper briquettes with a mean shatter resistance of 97.9%, exceeding the 91.3% mean shatter resistance of charcoal-starch briquettes (Lomunyak et al., 2024).

The structural performance of soaked waste paper is explained by cellulose hydration, particle adhesion, and mechanical interlocking during densification (Obi et al., 2022; Lomunyak et al., 2024). Briquette bonding occurs through adhesion, cohesion, solid bridges, capillary pressure, interfacial forces, and mechanical interlocking, and these mechanisms support the use of fibrous paper pulp as a structural binder (Obi et al., 2022). The increase in shatter resistance of charcoal-paper briquettes was attributed to tight interlocking between particles, confirming that hydrated paper fibers can maintain briquette integrity without commercial starch addition (Lomunyak et al., 2024).

Combustion Behavior and Health Impacts of Starch Additives

Starch binders increase briquette density and compaction strength, but stronger compaction can reduce aeration and increase ignition time (Lomunyak et al., 2024). Charcoal-paper briquettes ignited faster than charcoal-starch briquettes because paper-bound samples had higher volatile matter, lower density, and greater porosity for oxygen penetration (Lomunyak et al., 2024). Increasing starch proportion reduced burning rate because starch produced denser briquettes with stronger particle compaction and lower oxygen diffusion (Lomunyak et al., 2024).

Starch additives produced mixed indoor-air-quality outcomes across combustion studies (Glalah et al., 2024; Lomunyak et al., 2024; Oladosu et al., 2023). Charcoal-starch briquettes emitted CO values that increased from 96.7 ppm at 10% starch to 389 ppm at 20% starch, with a mean CO emission of 233.3 ppm for charcoal-starch samples (Lomunyak et al., 2024). Starch-bound cocoa pod and sawdust briquettes produced low PM2.5 values of 11.40 µg/m³ and 20.95 µg/m³, but their CO emissions of 17.90 ppm and 8.20 ppm exceeded the 6 ppm safe air-quality threshold used in the study (Glalah et al., 2024).

Cassava-starch briquettes can improve ignition and reduce particulate emissions when formulation and airflow are optimized (Oladosu et al., 2023). A flamboyant-pod/corn-cob/cassava-starch briquette at 30:52:18 recorded an ignition time of 185 s, compared with 250 s for charcoal, and reduced CO from 222 ppm to 196 ppm relative to charcoal (Oladosu et al., 2023). The same optimized cassava-starch briquette reduced PM2.5 from 0.21 ppm to 0.09 ppm and PM10 from 0.79 ppm to 0.33 ppm, indicating lower particulate release under the tested combustion condition (Oladosu et al., 2023). Starch-only biochar briquettes emitted higher CO₂, TVOC, and PM2.5 than red-clay/starch composite briquettes, showing that starch performance depends on the full binder system rather than starch addition alone (Suwankamnoed et al., 2025).

Comparative Analysis of Existing Briquette Machines and the Proposed AUTOBRIQ System

| Basis of Comparison | Cabaces et al. (2023) | Escalante and Alcaraz (2025) | Proposed AUTOBRIQ System |
|---|---|---|---|
| Raw materials | Agricultural wastes, animal manure as binder, and water. | Carbonized coconut shells, cassava starch, and water. | Prepared waste paper, charcoal fines, and water; paper pulp serves as the binder. |
| Material processing | Motorized mixing followed by hydraulic molding. | Prepared mixture passes through an auger and forming die, followed by automatic cutting. | Measured water dispensing, soaking and mixing in a shared container, gravity discharge, and compression molding. |
| Compression mechanism | Motor-driven hydraulic jack compresses material into a multi-hole mold. | Rotating auger compresses and extrudes material through a die. | Linear actuator compresses material into a partitioned pull-out mold, with load-cell feedback. |
| Output configuration | Twenty cylindrical briquettes measuring 50 mm in diameter and 50 mm in length. | Extruded briquettes automatically cut to a preset length. | Intended output of at least 20 rectangular briquettes per batch. |
| Power supply | Solar panels, battery bank, charge controller, and off-grid inverter. | A 5 hp single-phase electric motor drives the auger; a DC geared motor operates the cutter. | A 110 W solar panel charges the EcoFlow RIVER 2; an SMPS adjusted to 20 V supplies the mixer. |
| Control and monitoring | Variable-frequency drives, motor switches, and indicator lights. | A proximity sensor triggers the automatic cutting mechanism. | ESP32 coordinates stages using timers, water-flow readings, compression-force feedback, and position switches; touchscreen, PWA, and Micro-SD logging provide monitoring and records. |
| Operator interaction | Operators use the control-box switches and indicator lights. | Prepared mixture enters through the hopper; cutting occurs automatically when the sensor detects the required length. | Operators prepare and load materials, select settings, start the batch, and remove the filled mold. |

Table 4. Comparative Analysis of Existing Briquette Machines and the Proposed AUTOBRIQ System

Table 4 compares the proposed AUTOBRIQ system with existing briquetting machines. Both studies demonstrate mechanized briquette formation, while AUTOBRIQ is designed to coordinate water dispensing, shared-container soaking and mixing, gravity discharge, and compression through an ESP32-based control system. The comparison identifies differences in materials, forming mechanisms, power supply, and operator interaction. AUTOBRIQ’s actual production efficiency and product quality will be determined through prototype evaluation.

### Synthesis of the Reviewed Literature and Studies

The reviewed literature establishes that the problem addressed by AUTOBRIQ is both material and operational. In their study titled An overview of torrefied bioresource briquettes: quality-influencing parameters, enhancement through torrefaction and applications, Waheed et al. (2022) explain that loose biomass materials require densification to improve handling, storage, transport, and energy use. This supports the machine’s use of mechanical compression as the critical forming operation for transforming paper-charcoal slurry into a stable briquette. Similarly, in their study titled Combustion, physical, and mechanical characterization of composites fuel briquettes, Ibitoye et al. (2022) demonstrate that compaction pressure, blending ratio, and particle size influence briquette density, calorific value, shatter resistance, and water resistance. In their study titled Characteristics of charcoal briquettes from rice husk waste with compaction pressure variations, Yusuf et al. (2023) further show that compaction pressure affects density, stability, durability, combustion temperature, and combustion rate. These findings justify the study’s focus on controlled mechanical densification and measurable output quality rather than visual formation alone.

The literature also supports the use of waste paper as a natural binder for charcoal fines. In their study titled A review of biomass briquette binders and quality parameters, Obi et al. (2022) describe binder action as a major determinant of briquette cohesion, handling strength, and quality stability. In their study titled Innovative conversion of paper wastes into eco-friendly charcoal briquettes as a sustainable energy source and solution for school-based waste management, Felongco et al. (2025) show that paper-based briquettes can achieve improved compressive strength, longer burn duration, and lower porosity depending on paper type. This finding is reinforced by Lomunyak et al. (2024), whose study titled Characterization, optimization and emission analysis of manually-made charcoal dust briquettes with starch, paper and algae binders demonstrates that paper-bound charcoal dust briquettes can achieve high shatter resistance and acceptable fuel behavior. Genuino et al. (2022), in their study titled Evaluation of bio briquettes made from Musa acuminata Colla, Musa acuminata and Musa balbisiana Silk, further support the role of paper pulp as a viable binder when properly combined with biomass materials. Taken together, these studies justify soaked waste paper pulp as a cellulose-based binding medium that can interlock with charcoal fines under pressure without requiring synthetic adhesive.

Finally, the reviewed studies justify the transition from manual labor to a fully automated workflow. In their study titled Development of a low-cost briquetting system utilizing agri-waste materials as a renewable energy fuel, Cabaces et al. (2023) show that defined machine settings for mixing, compression, and briquetting time can standardize output and make production variables measurable. Escalante and Alcaraz (2025), in their study titled Performance of a locally developed automated screw-type charcoal briquetting machine, further demonstrate that machine parameters can be evaluated through capacity, efficiency, density, shatter resistance, and burn time. Beyond briquetting equipment, Alora and Amaya (2025), in their study titled Impact of lean principles on the productivity of bottling line process at XYZ manufacturing company, show that reducing motion, transportation, and overprocessing improves productivity, while Jesus et al. (2025), in their study titled Impact of adopting industry 4.0 practices and technologies on time and motion study: a pilot study revealing efficiency gains in a multi-operation workstation, establish the value of structured time-and-motion data in evaluating process improvements. These works support AUTOBRIQ’s transition from repetitive manual soaking, mixing, and pressing to an automated sequence that runs after material preparation, loading, and batch startup and ends after compression molding and press retraction, with manual mold removal and ambient air-drying remaining separate post-production activities.

## Chapter 3

METHODOLOGY

### Research Design

This study utilizes a quantitative applied developmental research design, specifically Type 1 Developmental Research. Following the standard academic taxonomy for developmental studies, Type 1 research is defined as the systematic study of the design, development, and rigorous evaluation of a specific, tangible product or tool. This classification is appropriate for this study because the primary objective is not to generate generalized theories, but to engineer a functional cyber-physical system (the solar-powered, fully automated paper-charcoal briquette machine) to address the practical, real-world inefficiencies of manual briquette production.

The methodology is inherently an applied research approach because it integrates mechanical hardware, low-power subsystems, a touchscreen Graphical User Interface (GUI), and a React-based monitoring dashboard into a localized solution intended to address a direct community problem. The development process requires strict standardization of material variables (the exact mass ratio of charcoal fines to paper waste and water volume) to ensure consistent conditions during compression molding. The machine’s architecture relies on closed-loop feedback sensors such as the YF-S201 for water flow and a load cell with an HX711 amplifier for compression force. These sensors are designed to reduce manual intervention and continuous operational oversight. Thermal drying is excluded from the machine cycle, and the briquettes are air-dried externally before fuel testing.

Furthermore, a quantitative method is utilized to evaluate the machine’s utility and directly answer the Statement of the Problem. The prototype’s performance is evaluated using numerical and statistical metrics. The system is tracked to measure man-hour consumption, overall throughput time, and standard production rate (units/hr) against an established manual baseline, together with batch yield relative to the target of at least 20 briquettes per batch. Finally, the physical output of the machine is quantitatively tested for structural integrity, heat retention, and burning duration, using store-bought wood charcoal as the comparative control. Therefore, this research design provides the necessary structured framework to develop the fully automated machine and evaluate its production efficiency and product quality.

### Flowchart of Research Design

Figure 2. Flowchart of Research Design

The flowchart illustrates the systematic, three-phase developmental pipeline executed in this study. The process initiates with System Design and Preparation, which focuses on establishing the controlled baseline parameters for the raw materials, specifically the paper waste, charcoal fines, and water. Following the material standardization, the study advances to Hardware and Software Development. This central engineering phase encompasses the physical assembly of the machine's hardware alongside the integration of its technological core, which includes the ESP32 subsystems, the Nextion touchscreen GUI and React PWA monitoring dashboard, the solar-charged portable power station, and the closed-loop sensory network. Once the cyber-physical prototype is fully constructed, it undergoes Testing and Evaluation. This final stage assesses the system's operational efficiency and physical output through targeted metrics, including man-hour consumption, production rate, dimensional and mass variance, a standardized 1-meter drop-test, combustion time and temperature, and a system usability Likert scale. The successful and sequential execution of these three phases culminates in the finalized output: the complete development of an embedded system architecture for the fully automated paper-charcoal briquette machine.

### Description of Research Instrument Used

Statement of the Problem 1:

To address the first Statement of the Problem regarding the key considerations in optimizing the utilization of paper waste and charcoal fines, the study relies on descriptive engineering synthesis and documented literature rather than physical measuring hardware. The primary method for this descriptive component is the analysis of existing waste management, briquette formulation, and energy demand studies, which establishes the socio-economic and technical considerations guiding the utilization of these materials as an alternative solid fuel source.

Statement of the Problem 2:

For the second Statement of the Problem, which investigates the physical demands, time consumption, and continuous operational oversight of the traditional manual briquette-making process, the primary research instruments are a digital stopwatch and structured observation logs. These tools are utilized to strictly record the continuous manual oversight, human labor hours, and operational bottlenecks of the traditional hand-pressed method, thereby establishing the quantitative baseline required for direct comparison against the fully automated system.

Statement of the Problem 3:

To determine the design and operational limitations of the proposed machine for the third Statement of the Problem, the research utilizes engineering documentation and prototype architectural logs. These descriptive instruments systematically record the mechanical boundaries, safety interlock constraints, and manual material feeding and mold removal requirements established during the hardware integration phase, ensuring that the system’s functional scope and its inability to extract water-insoluble toxins are explicitly mapped and defined.

Statement of the Problem 4:

Addressing the fourth Statement of the Problem regarding the machine’s effectiveness in production efficiency and product quality requires a matrix of quantitative hardware instruments. A digital stopwatch is used during production runs and combustion tests to track total burning duration, throughput time, and man-hour consumption, while a batch counter records batch yield against the target of at least 20 briquettes. Production rate is calculated from the recorded output and production time. Furthermore, an internal 300 kg S-type load cell with an HX711 amplifier is utilized to monitor compression force, a digital weighing scale and Vernier caliper measure the physical variance of the output, and an infrared thermometer measures surface temperatures to plot the heat retention of the final briquettes against store-bought charcoal.

Statement of the Problem 5:

Finally, to evaluate the acceptability of the fully automated system and its Graphical User Interface for the fifth Statement of the Problem, the study utilizes a pre-made questionnaire based on the ISO 25010 quality model. This Likert scale evaluation tool is distributed to target users following guided operational runs to quantitatively measure the system’s functional suitability, performance efficiency, interface usability, and reliability, specifically focusing on the clarity of the touchscreen status messages, safety fault alerts, and audio-visual cues.

### Mix Design

| Material Component | Initial Wet Mix Proportion | Final Dried Briquette Proportion |
|---|---|---|
| Paper Waste | 16.13% - 20% | 50% |
| Residual Charcoal Fines | 16.13% - 20% | 50% |
| Water | 67.7% - 60% | 0% |
| Total | 100% | 100% |

Table 5. Mix Design Proportions for Paper-Charcoal Briquettes

According to Sjöstrand, B., & Brolinson, A. (2022) in “Addition of polyvinylamine in chemi-thermomechanical pulp and kraft pulp and the effects on dewatering, strength, and air permeance”. Kraft pulp, the same material used to make the common paper bags, has the Water Retention Value (WRV) of 1.0-1.2, which is the ability of paper to hold water even through compression. According to Gupta, S. S., Prasad, R., Chakraborty, D., & Ebbing, M. (2026), the relevant Water Holding Capacity, the ability to hold water, of charcoal fines to the study is 2.0-3.0. The goal ratio of the mixture of research between paper and charcoal is 1:1, therefore if the paper of the same weight of the charcoal fines undergoes the process of soaking, then the ratio of the mixture after molding and compression would be 1.0:1.0:3.0 to 1.0:1.0:4.2 where charcoal fines:paper:water respectively. This will conclude the composition of the wet mixture to be:

The water content in the dried briquette should be negligible, therefore it will be 0%.

### Data Gathering Procedure

The data gathering procedure will formally commence with the identification and selection of the target beneficiaries, specifically fifteen selected end-users from Barangay Anglas and Barangay Poblacion, Cavinti, Laguna who have no prior experience operating the machine. Before any testing begins, the researchers will secure formal permission and obtain informed consent from these participants, ensuring they are fully aware of the study's purpose and their role in the evaluation process. This step is critical to ensure ethical compliance and to establish a cooperative environment for the subsequent instrumentation phases.

To determine the existing production constraints, the researchers will first gather baseline data directly from the participants as they perform the traditional manual briquette-making process. During this stage, the researchers will utilize digital timers and observation logs to conduct a time-and-motion study, strictly tracking the participants' total man-hour consumption and throughput time. This procedure yields the quantitative baseline data required to measure their current physical demands and operational efficiency before any technological intervention is introduced.

Following the manual baseline observation, the researchers will orient the participants on the operation of the fully-automated paper-charcoal briquette machine. The participants will then be asked to perform a guided production task, operating the system through the touchscreen GUI while manually loading materials and removing the mold. This allows the researchers to gather live time-and-motion data on the automated throughput and directly observe how effectively the users interact with the machine’s process status messages, safety fault alerts, and audio-visual cues.

Concurrently, as the participants operate the machine, the researchers will strictly record the necessary experimental data regarding the prototype's physical output. A sample of thirty machine-produced briquettes will be systematically collected from the users' batches for physical and volumetric variance tracking using a digital weighing scale and Vernier caliper. Furthermore, the researchers will execute the standardized one-meter drop-tests and thermal combustion tests using an infrared thermometer to quantitatively gather the structural and thermal performance data of the briquettes.

Upon the completion of the physical production cycle, the final phase of data gathering involves the direct administration of the survey instrument to the participants. The researchers will distribute the selected ISO/IEC 25010-based questionnaire to the fifteen target users to assess their firsthand experience regarding the system's functional suitability, interface learnability, and overall reliability. The researchers will assist any participants who require clarification regarding the survey items, after which the completed questionnaires will be collected, tallied, and subjected to the designated statistical treatments to finalize the evaluation of the system..

### Material Requirements

Central Processing, Network Routing, and Memory Units

| Component | Image | Description | Application |
|---|---|---|---|
| ESP32-WROOM-32D Dual-Core Core |  | A 32-bit symmetric multiprocessing architecture with dual execution cores. | Core 0 manages low-latency hardware ISRs and fail-safe interlocks. Core 1 handles the local networking stack, Wi-Fi AP, and 10 Hz WebSocket telemetry streaming. |
| Nextion NX8048T070 7.0” Smart UART Touchscreen LCD Assembly |  | Intelligent HMI panel with onboard flash memory and an independent graphics processor. | Provides batch-start controls, process status, water-volume and compression-force readings, and fault notifications through UART communication. |
| Physical SPI Micro-SD Card Module |  | Non-volatile edge-storage module interfacing via a high-speed external VSPI bus. | Ensures research data persistence by writing session arrays from the RAM cache to a physical FAT32 partition upon cycle completion. |

Table 6. Central Processing, Network Routing, and Memory Units

Solar-Charged Portable Power Station Infrastructure

| Component | Image | Description | Application |
|---|---|---|---|
| 110W Monocrystalline Photovoltaic Array |  | High-efficiency solar harvesting panel for varying irradiance. | Primary energy source; provides DC charging to maintain system power balance. |
| EcoFlow RIVER 2 Portable Power Station |  | 256 Wh LiFePO4 portable power station with a built-in MPPT solar charging controller supporting up to 110 W solar input, 300 W AC output, and a 12.6 V / 8 A (100 W max) DC car outlet. | Stores solar energy and supplies the mixer’s switching power supply through its AC outlet and the actuator and control circuits through its DC outlet. |
| UME S2415 Switching Power Supply |  | Switching power supply rated at 24 V, 15 A, with its output adjusted to 20 V DC. | Converts the power station’s AC output into the mixer’s DC supply. |

Table 7. Solar-Charged Portable Power Station Infrastructure

Electromechanical Actuation and Fluidics Modules

| Component | Image | Description | Application |
|---|---|---|---|
| Two 12 V Linear Actuators |  | Linear actuators rated at 1,500 N thrust, with 15 cm and 40 cm strokes and a rated speed of 7 mm/s. | The 15 cm actuator opens and closes the mixing-container trapdoor. The 40 cm actuator compresses the slurry in the mold and retracts after compression. Both are controlled through relays. |
| INGCO MXLI2001 20V Cordless Power Mixer |  | Lithium-ion paddle mixer with an aluminum gearbox, 0 to 650 rpm variable speed, and an M14 paddle thread, supplied at 20 V DC by the UME 24 V, 15 A switching power supply. | Mixes the paper, water, and charcoal into a homogeneous slurry in the shared container. Power is switched through the 30 A automotive relay. The mixer stops before gravity discharge. |
| 12 V DC Normally Closed Solenoid Water Valve |  | Electrically operated water valve that remains closed when de-energized. | Controls water inflow into the mixing container and closes when the preset volume is reached, based on flow-sensor readings. |
| 12V ERM Slurry-Settling Vibration Array |  | Electric vibration motor configured with an off-center balanced Eccentric Rotating Mass (ERM) assembly. | Executes a brief high-frequency vibration routine in the mold cavity to normalize slurry density before compaction. |
| Forced Airflow System |  | Environmental curing configuration using a brushless DC fan. | The fan vents moisture-saturated air out of the enclosure during drying. |
| 8-Channel 10 A Optocoupled Electromechanical Relay Module |  | Eight-channel relay board with optocoupled control inputs and electromechanical switching contacts. | Channels 1–4 control actuator direction, Channel 5 switches the water valve, and Channel 6 controls the automotive relay for the mixer. Channels 7–8 remain spare. |
| 12 V, 30 A Automotive Relay with Socket |  | Five-pin automotive relay with a 12 V coil and 30 A contact rating. | Switches the mixer’s 20 V supply when triggered through Channel 6 of the relay module. |

Table 8. Electromechanical Actuation and Fluidics Modules

Closed-Loop Sensory Feedback and Fail-Safe Interlocks

| Component | Image | Description | Application |
|---|---|---|---|
| 300 kg S-Type Load Cell & HX711 Amplifier |  | Strain-gauge load cell with a 300 kg rated capacity, connected to an HX711 24-bit ADC. | Monitors compression force and provides feedback for stopping the actuator at the preset compression-force threshold. |
| YFS-201G1 Hall-Effect Water Flow Sensor |  | Inline counting device with magnetic impeller and Hall-effect IC. | Counts flow pulses to calculate dispensed water volume and provide feedback for valve closure at the preset volume. |
| Roller Lever Micro Interlock Switches |  | Three roller-lever limit switches providing position feedback to the ESP32. | Switch 1 confirms the trapdoor is closed, Switch 2 confirms the compression actuator is fully retracted, and Switch 3 confirms the mold is fully seated. |
| Active Piezo Signalling Buzzers |  | High-decibel electronic modules generating fixed acoustic frequencies. | Decentralized three-module array for signaling process boundaries and manual operator prompts. |
| Emergency Shutdown Switch |  | Heavy-duty, lockable Red Mushroom E-Stop Button | Disconnects the switched 12 V actuator and trigger supply when pressed. The controller and touchscreen remain powered. |
| 1N4007 Flyback Diodes |  | Two diodes for suppressing switching-voltage spikes.. | Placed across inductive loads to safely dissipate high-voltage spikes, preventing electromagnetic interference |
| Electrical Enclosure |  | An IP65 Waterproof plastic control panel box | Protects the ESP32, motor drivers, buck converter, and relays from water splashes. |
| Logic Filter Capacitors |  | Ceramic and electrolytic decoupling (bypass) capacitors. | Placed across the VCC and GND pins of the ESP32 and sensitive ADC modules to filter out high-frequency power supply noise and stabilize sudden voltage drops |
| Resistors |  | Passive electrical components utilized for signal conditioning and current limiting. | Configured as pull-up or pull-down networks for the micro interlock switches and 1-Wire communication lines (DS18B20) to ensure stable logic levels |
| Common Ground Terminal Block and Wiring |  | Terminal block with 14 AWG power wiring and 22 AWG signal wiring. | Provides the common ground connection and connects power, control, and sensor circuits. |

Table 9. Closed-Loop Sensory Feedback and Fail-Safe Interlocks

### Software Requirements

| Name | Image | Description | Application |
|---|---|---|---|
| React.js + Vite (PWA) |  | Component-based JS library bundled for performance via Vite. | Primary HMI and Telemetry HUD, renders timers, warnings, and dashboards via offline GZIP files. |
| FreeRTOS (C++ Firmware) |  | RTOS supporting multi-threading, prioritization, and strict timing constraints. | ESP32 kernel; uses dual-core threading to separate safety ISRs (Core 0) from UI/network tasks (Core 1). |
| ESPAsyncWebServer & WebSockets |  | Asynchronous HTTP server and full-duplex communication protocol. | Manages local Wi-Fi AP and serves PWA; maintains 10Hz WebSocket telemetry for real-time UI updates. |
| LittleFS (Internal Storage) |  | Fail-safe filesystem for MCUs, featuring wear leveling and power-loss resilience. | Stores PWA assets (HTML/CSS/JS) on ESP32 internal flash for instant offline dashboard loading. |
| FAT32 Micro-SD & CSV Logger |  | Standard file system for removable media; stores structured data in CSV format. | Local edge database; logs runtime, dispensed water volume, and compaction force per batch for CSV download. |
| ArduinoJson |  | JSON serialization/deserialization library optimized for embedded C++. | Packages hardware telemetry on water volume, compression force, and process state into JSON strings for network broadcasting. |

Table 10. Software Requirements

### Statistical Treatment

Statement of the Problem 2:

To address the second Statement of the Problem regarding the physical demands and time consumption of the manual baseline, the study utilizes the arithmetic mean. This statistical tool calculates the average throughput time and human labor consumed across multiple manual production trials by dividing the sum of all recorded baseline data points by the total number of manual trials. Establishing this mean provides a reliable, mathematical baseline necessary for direct comparative analysis against the automated machine.

(Where  is the mean,  is each recorded observation,  is the sum of all recorded observations , and  is the total number of observations).

Statement of the Problem 4:

To determine the machine's effectiveness in improving production efficiency for the fourth Statement of the Problem, specific operational formulas are applied to the gathered time-and-motion data. The percentage reduction formulas are utilized to quantify changes in human labor and overall processing time separately when transitioning to fully automated production. Concurrently, the standard production rate formula evaluates the machine's throughput by calculating its standardized hourly yield, which is derived by dividing the total number of successfully produced briquettes by the total operational time in hours.

(Where  is the mean,  is each recorded observation,  is the sum of all recorded observations , and  is the total number of observations).

(Where  and  are the mean labor consumption of manual and fully automated production, respectively, measured in man-hours. Labor consumption is calculated by summing the recorded work hours of all operators.  and  are the mean elapsed throughput times, measured in minutes. Both comparisons will use equal production quantities).

(Where R is the production rate in units/hr, U is the total number of briquettes produced, and T is the total operational time in hours).

Further addressing the fourth Statement of the Problem concerning product quality and fuel performance, the study employs both the arithmetic mean and standard deviation. The arithmetic mean calculates the average burning duration and heat retention temperatures across all comparative test trials against the store-bought charcoal control. Meanwhile, the standard deviation is utilized to measure the physical consistency of the machine's output. By evaluating the mass and dimensional variation of the sampled briquettes, a lower standard deviation indicates greater consistency among the tested specimens.

(Where x is the mean, xi is each recorded observation, xi is the sum of all recorded observations , and n is the total number of observations).

(Where  is the standard deviation, xi is each individual briquette's value, x is the mean, and n is the population size).

Furthermore, to evaluate the structural integrity of the output for the fourth Statement of the Problem, the shatter resistance index or percentage weight loss formula is applied. Following a standardized drop-test, this formula calculates the percentage of mass lost by subtracting the weight of the largest remaining solid piece from the initial weight of the briquette, dividing that difference by the initial weight, and multiplying the quotient by one hundred. This specific mathematical treatment explicitly quantifies the physical durability of the final product, providing hard data to prove whether the machine-pressed briquettes can successfully resist shattering upon impact compared to standard charcoal.

(Where Wi is the initial weight of the briquette before the drop, and Wo is the weight of the largest remaining solid piece after the drop).

Finally, to evaluate the acceptability of the fully-automated system and its Graphical User Interface for the fifth Statement of the Problem, the study utilizes the weighted mean. This statistical treatment processes the distribution of responses gathered from the 5-point Likert scale ISO 25010 questionnaire. By dividing the sum of the products of the frequencies and their corresponding weights by the total number of respondents, the resulting weighted mean is matched against a standardized verbal interpretation scale to quantitatively conclude the system's functional suitability, performance efficiency, usability, and reliability.

(Where x is the mean, xi is the sum of all recorded data points, and n is the total number of trials).

### Design Project Flow

The Design Project Flow outlines the engineering execution required to build the fully-automated paper-charcoal briquette machine. This section details the integration of the mechanical hardware, low-power microcontrollers, and software logic that drives the automated batch-production process. To ensure a reproducible development cycle, the system design is structured into three core components: the Block Diagram, the System Architecture, and the Prototype Design.

#### Block Diagram

Figure 3. Block Diagram of the Hardware Flow

The block diagram shows the hardware flow of the fully automated paper-charcoal briquette production system with a graphical user interface. The power section starts from the solar panel, which charges the EcoFlow RIVER 2 power station through its built-in MPPT charge controller. Energy is stored in the power station’s internal battery and supplied to the 12 V power rail and switching power supply. The emergency stop switch disconnects the switched 12 V supply during an emergency.

The 12 V power rail supplies the water control valve, vibration motor, exhaust fan, and linear actuators, while the switching power supply provides power to the mixing motor. The relay module controls the ON/OFF loads and the direction of the linear actuators, while the automotive relay switches power to the mixing motor. These actuators perform the main automated production processes.

The ESP32 controller receives feedback from the sensor inputs, which include the water flow sensor, compression force sensor, and position limit switches. These sensors monitor the dispensed water volume, compression force, and the positions of the trapdoor, compression actuator, and mold during operation.

The ESP32 also connects to the human-machine interface touchscreen display, buzzer alert, and storage hardware micro-SD card module. The touchscreen provides user control and system monitoring, the ESP32 activates the buzzer to provide audio alerts for process notifications and system faults, and the micro-SD card stores batch logs and configuration files. Overall, the block diagram shows how the power supply, protection system, controller, sensors, actuators, interface, and storage hardware are connected to produce the final paper-charcoal briquette output.

#### Process Flowchart

To ensure deterministic control and operator safety, the system’s operational logic is divided into five modular phases governed by the ESP32 microcontroller’s finite state machine. Operator involvement is limited to selecting the water level, loading the paper and crushed charcoal when prompted, starting the batch, and removing the container to dispense the compressed mixture; every stage between batch start and batch completion transitions automatically through timers, sensor feedback, and safety interlocks.

Module 1: System Initialization, Loading, and Batch Start

The system begins in the Initialization state upon receiving power from the solar-charged portable power station and then guides the operator through material loading.

Figure 4. Process Flow: Initialization, Loading, and Batch Start

Figure 4 illustrates the boot sequence, diagnostics, and two-step loading interface of the ESP32 finite state machine. Upon receiving power, the system mounts the internal LittleFS flash partition and the external FAT32 SD card, boots the NVS registry for state retention, broadcasts the Wi-Fi Access Point hosting the React PWA dashboard, and initializes the Nextion touchscreen. A self-test then confirms that the enclosure interlocks are closed, both actuators are at their home positions, and the power station output is present; any failure routes to a fault state with an on-screen alert and buzzer alarm. The first loading screen prompts the operator to load the shredded waste paper into the mixing container and to set the water level on a vial-shaped selector that ranges from 30% to 100% of the container’s water capacity and is preset to 100%. The Next button stays visible while the operator adjusts the level and loads the paper. Once it is pressed, the paper prompt slides to the left and a second prompt appears on its right, asking the operator to load the crushed charcoal into the hopper. The automated cycle begins when the operator presses the final START button.

Module 2: Water Dispensing and Mixing Phase

This phase adds the selected volume of water and mixes the paper, water, and charcoal into a uniform slurry.

Figure 5. Process Flow: Water Dispensing and Mixing State

Figure 5 delineates the logic flow for preparing the mixture. The ESP32 energizes the relay of the normally closed 12V solenoid water valve, and a hardware interrupt service routine tied to the YF-S201 Hall-Effect sensor counts flow pulses until the water volume that corresponds to the selected level is dispensed into the mixing container. The valve then closes and the ESP32 switches on the relay of the 20V INGCO power mixer, which receives its 20 V DC supply from the switching power supply adjusted to 20 V through the 30 A automotive relay, for a preset mixing time. The hydrated paper fibers break down into pulp while the crushed charcoal loaded through the hopper is blended in, producing a homogeneous paper-charcoal slurry.

Module 3: Mixture Discharge Phase

This module empties the mixing container into the compression box.

Figure 6. Process Flow: Mixture Discharge State

Figure 6 details the algorithmic progression of the Mixture Discharge Phase. When the mixing time ends, the mixer stops. After the coast-down pause, the ESP32 retracts the 12V 15 cm linear actuator through its double-relay setup, opening the base platform of the mixing container. While the platform is open, the mixer remains OFF, and ERM vibration motor 1, mounted on the mixing container, shakes the remaining mixture down so that the container empties completely into the compression box. When the discharge time ends, ERM 1 stops and the 15 cm actuator extends to close the platform. SW1 confirms trapdoor closure before the system proceeds to leveling and compression. No operator confirmation is required between water dispensing, mixing, and discharge.

Module 4: Leveling and Compression Phase

This is the core densification stage requiring strict physical sensor and actuator oversight.

Figure 7. Flowchart: Leveling and Compression Stage

Figure 7 maps the control logic and fail-safe interlocks of the densification stage. ERM vibration motor 2, mounted on the compression box, levels the mixture for a preset time to normalize its density. Once the leveling time is reached, ERM vibration motor 2 stops. After SW1 confirms trapdoor closure and SW3 confirms mold seating, the ESP32 extends the 40 cm linear actuator through its double-relay setup, pressing the leveled mixture into the partitioned pull-out mold while polling the HX711 24-bit ADC connected to the 300 kg S-type load cell on the compressor block. The ESP32 turns the extend relay OFF once the preset compression-force threshold is reached. This threshold will be established through calibration before performance testing. Throughout this stage, SW1 and SW3 are monitored. An unexpected trapdoor opening, mold removal, invalid force reading, or compression timeout stops the production loads and routes the system to fault handling.

Module 5: Retraction, Unloading, and Batch Completion

The final phase releases the compressed mixture to the operator and records the batch.

Figure 8. Flowchart: Retraction, Unloading, and Batch Completion Stage

Figure 8 outlines the unloading logic and the edge data logging operations. After compression, the 40 cm actuator retracts through its double-relay setup until SW2 confirms the compressor’s home position, after which the retract relay turns OFF. The batch record, including runtime, selected water level, dispensed water volume, and peak compaction force, is saved to the SD card as a CSV row and pushed to the PWA dashboard over WebSocket. The buzzer then sounds and the touchscreen prompts the operator to remove the pull-out mold containing the formed briquettes. The briquettes are air-dried outside the machine. Once SW3 detects mold removal, the system returns to Module 1. Another batch requires an empty, correctly seated mold and a new START command. No thermal drying is performed by the machine.

System Architecture

Figure 9: System Architecture of the Fully Automated Paper-Charcoal Briquette Machine

The architecture utilizes a closed-loop control system driven by an ESP32 dual-core microcontroller running a FreeRTOS state machine. The system integrates real-time sensor feedback, parallel processing, and an local edge framework to ensure consistent production quality, operator safety, and autonomous data logging.The system architecture is divided into four distinct hierarchical layers:

Layer 1: The Physical Plant (Mechanical Level): Comprises the electromechanical hardware, including an INGCO MXLI2001 20 V power mixer, a 15 cm linear actuator for the mixing-container trapdoor, a 40 cm linear actuator that compresses the mixture into a partitioned pull-out mold, two ERM vibration motors for discharge and leveling, and the water solenoid valve. It is powered by a 110 W solar panel charging an EcoFlow RIVER 2 portable power station (256 Wh LiFePO4, 12.6 V DC car outlet). The power mixer receives its 20 V DC supply through a UME 24 V, 15 A switching power supply adjusted to 20 V and connected to the power station’s AC outlet.

Layer 2: The Bridge (Sensor & Switching Level): Acts as the intermediary interface. An array of sensors, including the YF-S201 flow meter, a 300 kg S-type load cell connected through the HX711, and three position limit switches, captures physical metrics and component positions. An optocoupled 8-channel relay module switches the 12 V actuator, valve, and vibration-motor circuits in response to ESP32 control signals, with two relay channels per linear actuator setting its direction. The mixer’s 20 V supply is switched through a 30 A automotive relay triggered by the relay module.

Layer 3: Edge Computing (Firmware Level): The ESP32 microcontroller acts as the central brain. It handles parallel task execution, sensor data processing, logic gating, and localized data storage via a FAT32 Micro-SD card module for batch metric logging.

Layer 4: Human-Machine Interface (Operator Level): Operator interaction is facilitated through two primary interfaces: a localized Nextion NX8048T070 7.0" touchscreen via UART and a React Progressive Web App (PWA) hosted on the ESP32’s local Wi-Fi Access Point. The touchscreen provides batch-start controls, process status, and fault notifications, while the PWA provides real-time monitoring and CSV downloads over the local wireless connection.

#### Prototype Design

Figure 10. Isometric View of the Prototype Design for the Fully-Automated Paper-Charcoal Briquette Machine

Figure 11. Front View of the Prototype Design for the Fully-Automated Paper-Charcoal Briquette Machine

Figure 12. Rear View of the Prototype Design for the Fully-Automated Paper-Charcoal Briquette Machine

Figure 13. Left and Right Side Views of the Prototype Design for the Fully-Automated Paper-Charcoal Briquette Machine

Figure 14. Top View of the Prototype Design for the Fully-Automated Paper-Charcoal Briquette Machine

Figure 15. Expected Output Briquette Size

### Specimen Details

To systematically conduct the research instruments, the evaluations should be conducted with maintaining three categories of specimens consistently, mainly the Primary test specimens, Raw material specimens, and Control specimens.

The tests to be done to answer the statement of the problems concerns the 30 end products produced by the fully-automated paper-charcoal briquette production. These specimens will undergo evaluation on dimensions, structure, and fuel performance. The stated evaluation includes: dimensional variance tracking, mass measurement, standardized drop test, burning time measurement, and burning temperature measurement. These 30 specimens stated above are the Primary test specimens.

Raw material specimens on the other hand, directly affects the primary test specimens as they are the inputs for the fully-automated paper-charcoal briquette production.. These are strictly limited to: charcoal fines, waste paper, and water.  Control specimens will serve as comparative baselines for evaluating the products of the fully automated paper-charcoal briquette machine and should be surpassed by the new technology. These are the common store-bought charcoal and the manually produced paper-charcoal briquette.

### Multiple Constraints

The execution of this developmental research and the operation of the fully-automated machine are strictly bounded by the following physical, operational, and material constraints:

- Environmental and Power Constraints

| Constraint Name | Description |
|---|---|
| Off-Grid Infrastructure Constraint | To ensure deployment viability in resource-constrained areas, the entire hardware architecture is constrained to operate entirely off-grid, drawing its operational energy exclusively from the 110 W solar panel and EcoFlow RIVER 2 power station. |
| Weather Dependency Constraint | The system’s solar charging depends on local solar irradiance. Prolonged overcast weather may reduce the energy generated by the 110 W solar panel, limiting recharging of the EcoFlow RIVER 2 power station and interrupting operation when its stored energy is insufficient. |

Table 11. Environmental and Power Constraints

- Material and Output Constraints

| Constraint Name | Description |
|---|---|
| Material Constraint | The fully-automated pipeline is restricted to processing exactly three input materials: paper waste, residual charcoal fines, and water. |
| Impurities Constraint | The system is incapable of extracting any water-insoluble impurities in the paper waste prior to the soaking phase. |
| Shape Constraint | The physical geometry of the final output is strictly limited to a solid rectangular prism designed specifically for cooking. |
| Size Constraint | The internal molding and linear actuator assembly is engineered to produce a single, standardized briquette size. |

Table 12. Material and Output Constraints

- Process and Mechanical Constraints

| Constraint Name | Description |
|---|---|
| Batch Operation Constraint | The system is constrained to a discrete batch-processing workflow. A new batch requires completion of the previous cycle, removal of the filled mold, and insertion of an empty, correctly seated mold. |
| Viscosity and Torque Constraint | The precision of load-cell readings obtained through the HX711 is constrained by physical noise; vibrations from the installed vibration motor or other moving components may induce minor reading fluctuations. |
| Actuator Stroke Constraint | The fixed stroke length of the linear actuator limits the maximum volume of wet slurry that can be compressed in a single cycle without risking mechanical binding. |
| Compaction Limit Constraint | The densification of the briquettes is strictly hard-capped by the maximum mechanical thrust of the installed linear actuator and the structural integrity of its localized mounting brackets. |

Table 13. Process and Mechanical Constraints

- Hardware and Sensor Constraints

| Constraint Name | Description |
|---|---|
| Vibration Interference Constraint | The precision of the HX711 load cell is constrained by physical noise; heavy vibrations generated by the pulverizer and mixing motors may induce minor reading fluctuations during simultaneous motor operation. |
| Volumetric Resolution Constraint | The automated water intake is constrained by the physical resolution of the flow meter, limiting the system from dispensing precise microliter-level measurements. |

Table 14. Hardware and Sensor Constraints

- Embedded System and Interface Constraints

| Constraint Name | Description |
|---|---|
| Microcontroller Processing Constraint | The system is constrained to the computational limits of the ESP32-WROOM-32D, utilizing a dual-core architecture to strictly separate asynchronous network tasks from deterministic physical control logic to prevent system latency. |
| Storage Media Constraint | Localized data logging is strictly limited to an external FAT32 formatted micro-SD card utilizing an append-only Comma-Separated Values (CSV) structure, as the microcontroller lacks the RAM required to run standard relational database engines (SQL). |
| HMI Display Constraint | The physical Human-Machine Interface (HMI) is constrained to a 7-inch touchscreen, limiting the on-machine rendering of complex, high-density analytical graphs and requiring the PWA for deeper data extraction. |

Table 15. Embedded System and Interface Constraints

- Software and Network Constraints

| Constraint Name | Description |
|---|---|
| Network Connectivity Constraint | The system operates strictly as a localized Edge IoT node; it does not possess cellular or wide-area network (WAN) connectivity, entirely preventing the transmission of telemetry data to remote internet or cloud servers. |
| Wireless Client Constraint | The localized Software Access Point (SoftAP) broadcasted by the microcontroller is constrained to reliably support a maximum of 4 to 5 simultaneous connected mobile devices for monitoring the Progressive Web App (PWA) dashboard. |
| Web Application Dependency Constraint | Because the system operates entirely off-grid, the PWA must be fully self-contained within the microcontroller's internal LittleFS partition, explicitly prohibiting the use of external Content Delivery Networks (CDNs) for fonts, icons, or CSS frameworks. |

Table 16. Software and Network Constraints

- Human-Machine Operational Constraints

| Constraint Name | Description |
|---|---|
| Loading Constraint | The system relies on the operator to manually feed the raw biomass materials (shredded waste paper and charcoal fines) into the designated hoppers, as the machine lacks an automated bulk material intake mechanism. |
| Decision Constraint | The operator checks the preset operating parameters and starts the batch through the touchscreen GUI. Subsequent manufacturing stages transition automatically using timers, sensor feedback, and safety interlocks. |
| Notice & Alert Constraint | The system uses visual touchscreen messages and audible buzzer alerts to indicate process status, batch completion, and system faults. |

Table 17: Human-Machine Operational Constraints

References

- Agustinus, Allo, R., Tambing, E., Rantepulung, S., & Kaiway, M. R. (2025). Optimization of hole variations in honeycomb charcoal briquettes from corn waste for enhanced combustion efficiency. G-Tech: Jurnal Teknologi Terapan, 9(2), 738-747. https://doi.org/10.70609/gtech.v9i2.6639

- Akogun, O. A., & Waheed, M. A. (2022). Development and performance evaluation of a piston type hydraulically operated briquetting machine with replaceable moulds. Agricultural Engineering International: CIGR Journal, 24(1), 113-127. https://cigrjournal.org/index.php/Ejounral/article/view/6997

- Alabi, O. O., Adeyi, T., & Ekun, S. (2023). Analyzing energy performance and assessing dry moisture content of briquettes through numerical investigations. Engineering and Technology Journal, 42(1), 173–182. https://doi.org/10.30684/etj.2023.143782.1608

- Alfanz et al. (2023). Smart farm agriculture design by applying a solar power plant. https://doi.org/10.25077/jnte.v12n2.1085.2023

- Alora, L. B., & Amaya, F. L. (2025). Impact of lean principles on the productivity of bottling line process at XYZ manufacturing company. International Journal of Academic Research in Business and Social Sciences, 15(4). https://doi.org/10.6007/IJARBSS/v15-i4/25188

- Anonas, S. D. S., Eugenio, F. D. T., Flores, B.-H. F., Balite, P. H. M., Tomacruz, J. G. T., Limjuco, L. A., & Ocon, J. D. (2023). From waste to renewable energy: A policy review on waste-to-energy in the Philippines. *Sustainability, 15*(17), 12963. https://doi.org/10.3390/su151712963

- Arabi, & Dahmardeh Ghalehno. (2022). Suitability of thin-layer drying models for halogen lamp drying of sugarcane bagasse. https://doi.org/10.15376/biores.18.1.1052-1071

- Baccay, O. T. (2026). DENR turns paper waste into eco-friendly charcoal briquettes in Batanes. Philippine Information Agency. https://pia.gov.ph/news/denr-turns-paper-waste-into-eco-friendly-charcoal-briquettes-in-batanes/

- Barkalov, A., Titarenko, L., Bieganowski, J., & Krzywicki, K. (2024). Basic approaches for reducing power consumption in finite state machine circuits-A review. *Applied Sciences, 14*(7), 2693. https://doi.org/10.3390/app14072693

- Burigsay, H. L. (2025). Developing sustainable solid waste management enterprise in Pudtol, Apayao: A feasibility study and program design. *International Journal for Multidisciplinary Research, 7*(3). https://www.ijfmr.com/research-paper.php?id=44958

- Cabaces, D., et al. (2023). Development of a low-cost briquetting system utilizing agri-waste materials as a renewable energy fuel. Batangas State University. https://ojs.batstate-u.edu.ph/index.php/IRJIEST/article/view/94

- Castillo-Dida, V. (2024). The link between electricity prices and carbon emissions.https://journals.uplb.edu.ph/index.php/EDJ/article/view/1266

- Carrión-León, D. I., Lopez-Ramos, M. P., Santillan-Valdiviezo, L. G., Tanguila-Tapuy, D. S., Morocho-Santos, G. M., Moyano-Arias, R. J., Yautibug-Apugllón, M. E., & Chacón-Luna, A. E. (2025). Evaluating interaction capability in a serious game for children with ASD: An operability-based approach aligned with ISO/IEC 25010:2023. *Computers, 14*(9), 370. https://doi.org/10.3390/computers14090370

- Chipangura et al. (2024). Fabrication of briquettes from charcoal fines using tannin formaldehyde resin as a binder. https://doi.org/10.29333/ejosdr/14125

- Dragusanu, V., Lunguleasa, A., & Spirchez, C. (2022). Evaluation of the physical, mechanical, and calorific properties of briquettes with or without a hollow made of wheat (Triticum aestivum L.) straw waste. Applied Sciences, 12(23), Article 11936. https://doi.org/10.3390/app122311936

- Drutchas, J., & Eppinger, S. (2022). Guidance on application of agile in combined hardware and software development projects. *Proceedings of the Design Society, 2*, 151-160. https://doi.org/10.1017/pds.2022.16

- Dumagan, J. C., & Abrigo, M. R. M. (2022). Rational choices and welfare changes in Philippine family energy demand. De La Salle University.

- Dumrique, M.-C. G., Capili, J. M. F., Macugay, J. T., Pasilabban, E. T., & Alvarez, J. P. (2024). Production and evaluation of carbonized rice straw and corn husk briquettes as a sustainable and alternative fuel for cooking. *Journal of Pure and Applied Sciences, 4*(2). https://csu.org.ph/jpas/article/download/54/47/191

- Escalante, & Alcaraz. (2025). Performance of a locally developed automated screw-type charcoal briquetting machine. https://scienggj.org/2025/SciEnggJ%202025-vol18-no01-p44-50-Escalante%20and%20Alcaraz.pdf

- Fagcang, H., Stobart, R., & Steffen, T. (2022). A review of component-in-the-loop: Cyber-physical experiments for rapid system development and integration. *Advances in Mechanical Engineering, 14*(8). https://doi.org/10.1177/16878132221109969

- Felongco, W. S., Hemongala, E. E. M., Remotin, P. I. L., & Villacarlos, R. F. (2025). Innovative conversion of paper wastes into eco-friendly charcoal briquettes as a sustainable energy source and solution for school-based waste management. Ignatian International Journal for Multidisciplinary Research, 3(11), 827–841. https://doi.org/10.5281/zenodo.17762647

- Figueiroa, V., & Torres, J. P. N. (2022). Simulation of a small smart greenhouse. *Designs, 6*(6), 106. https://doi.org/10.3390/designs6060106

- Gebremariyam, E. Z., Demeke, E. D., Mengistu, W. M., & Mekonnen, Y. S. (2025). Briquette production from industrial and fruit wastes as a clean fuel for versatile applications. Scientific Reports, 15, Article 40126. https://doi.org/10.1038/s41598-025-23806-1

- Genuino, A. F. R., Tancuan, J. M. M., et al. (2022). Evaluation of bio briquettes made from Musa acuminata Colla, Musa acuminata and Musa balbisiana Silk. De La Salle University.

- Glalah et al. (2024). Binder-type effect on physico-mechanical, combustion and emission properties of biochar briquettes. https://doi.org/10.1371/journal.pone.0306827

- Gupta, S. S., Prasad, R., Chakraborty, D., & Ebbing, M. (2026). Pelletizing poultry litter with biochar: A strategy to enhance poultry litter utilization. Waste and Biomass Valorization. https://www.researchgate.net/publication/402101786_Pelletizing_Poultry_Litter_with_Biochar_A_Strategy_to_Enhance_Poultry_Litter_Utilization

- Habib et al. (2023). Estimating shifts in fuel stacking among solid biomass fuels and LPG. https://doi.org/10.21203/rs.3.rs-2674609/v1

- Ibitoye, A. R., et al. (2022). Combustion, physical, and mechanical characterization of composites fuel briquettes. International Journal of Renewable Energy Development, 11(3), 859-868. https://ijred.cbiore.id/index.php/ijred/article/view/41290

- International Organization for Standardization & International Electrotechnical Commission. (2023). *ISO/IEC 25010:2023 Systems and software engineering - Systems and software Quality Requirements and Evaluation (SQuaRE) - Product quality model*. https://www.iso.org/standard/78176.html

- Jesus, C., Amorim, I., Pontes, E., Gonçalves, S., Moradbeikie, A., & Lopes, S. (2025). Impact of adopting industry 4.0 practices and technologies on time and motion study: a pilot study revealing efficiency gains in a multi-operation workstation. Production & Manufacturing Research, 13(1). https://www.tandfonline.com/doi/full/10.1080/21693277.2025.2476436

- Kah’fi, M. A., Santoso, D., Mansyur, N. I., Ismandari, T., Jannah, N., & Egra, S. (2025). The effect of briquette shape variation on the combustion quality of rice husk waste briquettes. J-PEN Borneo: Jurnal Ilmu Pertanian, 8(1). https://doi.org/10.35334/vemhhe55

- Kalli et al. (2022). Subsidized LPG scheme and the shift to cleaner household energy use. https://doi.org/10.3390/su14042450

- Kebede, T., Berhe, D. T., & Zergaw, Y. (2022). Combustion characteristics of briquette fuel produced from biomass residues and binding materials. Journal of Energy, 2022, Article 4222205. https://doi.org/10.1155/2022/4222205

- Lomunyak et al. (2024). Characterization, optimization and emission analysis of manually-made charcoal dust briquettes with starch, paper and algae binders. https://doi.org/10.1016/j.heliyon.2024.e40991

- Łaska, & Ige. (2023). A review: Assessment of domestic solid fuel sources in Nigeria. https://doi.org/10.3390/en16124722

- Magbanua, S. A. (2026). LPG use drops by 30% as Filipinos shift to firewood in summer.https://tribune.net.ph/2026/04/13/lpg-use-drops-30-filipinos-shift-to-firewood-in-summer

- Malovanyy et al. (2023). The use of binders of natural origin to improve the technology of creating fuel briquettes from wood waste. https://doi.org/10.12911/22998993/171901

- Meniano, S. (2026). DENR-8 pushes eco-friendly charcoal as LPG prices drive poaching. Philippine News Agency.https://www.pna.gov.ph/articles/1273864

- Mentes, D., Jordan, A., Farkas, L., Muranszky, G., Fiser, B., Viskolcz, B., & Poliska, C. (2024). Evaluating emissions and air quality implications of residential waste incineration. *Scientific Reports, 14*, 21314. https://doi.org/10.1038/s41598-024-72173-w

- Mofokeng, N. N., Madikizela, L. M., Tiggelman, I., & Chimuka, L. (2024). Chemical profiling of paper recycling grades using GC-MS and LC-MS: An exploration of contaminants and their possible sources. *Waste Management, 189*, 148-158. https://doi.org/10.1016/j.wasman.2024.08.014

- Montiel et al. (2023). Algal biomass (Green algae) as a binding agent for the production of eco-briquettes from water hyacinth. De La Salle University.

- Murakami, K., & Sato, S. (2024). Optimization of mixing ratios of binders and organic matter for charcoal briquette using biochars derived from water hyacinth. Solid Fuel Chemistry, 58(3), 226-231. https://doi.org/10.3103/S0361521924700022

- Mwamlima et al. (2023). Efficiency of place-based innovated briquette making technologies for sustainable cooking energy. https://doi.org/10.37425/eajsti.v4i3.710

- Nkuriyingoma, O., Özdemir, E., & Sezen, S. (2022). Techno-economic analysis of a PV system with a battery energy storage system for small households: A case study in Rwanda. Frontiers in Energy Research, 10. https://doi.org/10.3389/fenrg.2022.957564

- Obi, O. F., Pecenka, R., & Clifford, M. J. (2022). A review of biomass briquette binders and quality parameters. In Energies (Vol. 15, p. 2426). https://doi.org/10.3390/en15072426

- Oladosu, K. O., Babalola, S. A., Kareem, M. W., Ajimotokan, H. A., Kolawole, M. Y., Issa, W. A., Olawore, A. S., & Ponle, E. A. (2023). Optimization of fuel briquette made from bi-composite biomass for domestic heating applications. Scientific African, 21, Article e01824. https://doi.org/10.1016/j.sciaf.2023.e01824

- Pantuhan, G. P., Servinas, M. O., & Hassan, V. A. (2024). Energy challenges and strategies by rural communities in a southern Philippine city. *Journal of Health Research and Society, 3*(2), 66-77. https://doi.org/10.34002/jhrs.v3i2.59

- Pniel, M. V. H. (2026). Automation of moisture level measurement in charcoal briquettes. E3S Web of Conferences. https://doi.org/10.1051/e3sconf/202668701012

- Qi et al. (2022). Improving combustion technology for cooking activities for pollutant emission reduction and carbon neutrality. https://doi.org/10.3390/atmos13040561

- Sekyere, C. K. K., Opoku, R., Asaaga, B., Baah, B., Andoh, P. Y., Obeng, G. Y., & Agbogla, J. (2025). Techno-environmental assessment of the fuel properties of a variety of briquettes for biomass boiler applications. Cleaner Energy Systems, 10, Article 100185. https://doi.org/10.1016/j.cles.2025.100185

- Sjöstrand, B., & Brolinson, A. (2022). Addition of polyvinylamine in chemi-thermomechanical pulp and kraft pulp and the effects on dewatering, strength, and air permeance. BioResources, 17(3), 4098–4115. https://bioresources.cnr.ncsu.edu/resources/addition-of-polyvinylamine-in-chemi-thermomechanical-pulp-and-kraft-pulp-and-the-effects-on-dewatering-strength-and-air-permeance/

- Suwankamnoed, S., Imsuwan, P., Tilokkarn, W., Meksiriporn, B., & Sutjaritvorakul, T. (2025). Characterization and proximate analysis of composite biochar briquette from oil palm frond and red clay. Journal of Ecological Engineering, 26(11), 45-54. https://doi.org/10.12911/22998993/207634

- Tesfaye, A., et al. (2023). Development and evaluation of a coffee husk biomass briquette machine. Indian Journal of Sustainable Research. https://www.researchgate.net/publication/373444645_Development_and_Evaluation_of_a_Coffee_Husk_Biomass_Briquette_Machine

- Thongsan, T., Angkawisittpan, N., Thung-Od, K., & Techasarntikul, N. (2023). Smart data management for charcoal briquettes manufacturing using IoT technologies. Materials Today: Proceedings. https://doi.org/10.64289/iej.25.03A11.9182674

- Tuan et al. (2023). Study on drying of bitter gourd slices based on halogen dryer. https://doi.org/10.17221/97/2022-rae

- Usaka, E., Edeoja, A. O., & Ibrahim, J. S. (2023). Effect of process parameters on the quality of bio-coal briquette produced from Prosopis africana pods. European Journal of Science, Innovation and Technology, 3(6), 264-275. https://ejsit-journal.com/index.php/ejsit/article/view/334

- Waheed, A., et al. (2022). An overview of torrefied bioresource briquettes: quality-influencing parameters, enhancement through torrefaction and applications. Bioresources and Bioprocessing, 9, 23. https://link.springer.com/article/10.1186/s40643-022-00608-1

- Waluyo et al. (2022). An effect of blending ratio on mechanical and thermal properties of sawdust-cocoa pod briquettes. https://doi.org/10.18280/ijdne.170417

- wondimagegn, G. H., & Edao, S. A.  (2023). Adaptation and Evaluation of Pellet Press Briquette Machine. https://iijsr.com/data/uploads/7481.pdf

- Yusuf et al. (2023). Characteristics of charcoal briquettes from rice husk waste with compaction pressure variations. https://doi.org/10.12911/22998993/159966

APPENDICES
