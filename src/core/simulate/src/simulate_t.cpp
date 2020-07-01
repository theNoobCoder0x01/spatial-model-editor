#include <sbml/SBMLDocument.h>
#include <sbml/SBMLReader.h>
#include <sbml/SBMLWriter.h>

#include <QFile>
#include <algorithm>

#include "catch_wrapper.hpp"
#include "model.hpp"
#include "sbml_test_data/very_simple_model.hpp"
#include "simulate.hpp"
#include "utils.hpp"

SCENARIO("Simulate: very_simple_model, single pixel geometry",
         "[core/simulate/simulate][core/simulate][core][simulate][pixel]") {
  // import model
  std::unique_ptr<libsbml::SBMLDocument> doc(
      libsbml::readSBMLFromString(sbml_test_data::very_simple_model().xml));
  libsbml::SBMLWriter().writeSBML(doc.get(), "tmp.xml");
  model::Model s;
  s.importSBMLFile("tmp.xml");

  // import geometry & assign compartments
  QImage img(1, 3, QImage::Format_RGB32);
  QRgb col1 = QColor(12, 243, 154).rgba();
  QRgb col2 = QColor(112, 243, 154).rgba();
  QRgb col3 = QColor(212, 243, 154).rgba();
  img.setPixel(0, 0, col1);
  img.setPixel(0, 1, col2);
  img.setPixel(0, 2, col3);
  img.save("tmp.bmp");
  s.getGeometry().importGeometryFromImage(QImage("tmp.bmp"));
  s.getGeometry().setPixelWidth(1.0);
  s.getCompartments().setColour("c1", col1);
  s.getCompartments().setColour("c2", col2);
  s.getCompartments().setColour("c3", col3);
  // check we have identified the compartments and membranes
  REQUIRE(s.getCompartments().getIds() == QStringList{"c1", "c2", "c3"});
  REQUIRE(s.getMembranes().getIds() ==
          QStringList{"c1_c2_membrane", "c2_c3_membrane"});

  // check fields have correct compartments & sizes
  geometry::Field *fa1 = s.getSpecies().getField("A_c1");
  REQUIRE(fa1->getCompartment()->getId() == "c1");
  REQUIRE(fa1->getId() == "A_c1");
  geometry::Field *fb1 = s.getSpecies().getField("B_c1");
  REQUIRE(fb1->getCompartment()->getId() == "c1");
  REQUIRE(fb1->getId() == "B_c1");
  geometry::Field *fa2 = s.getSpecies().getField("A_c2");
  REQUIRE(fa2->getCompartment()->getId() == "c2");
  REQUIRE(fa2->getId() == "A_c2");
  geometry::Field *fb2 = s.getSpecies().getField("B_c2");
  REQUIRE(fb2->getCompartment()->getId() == "c2");
  REQUIRE(fb2->getId() == "B_c2");
  geometry::Field *fa3 = s.getSpecies().getField("A_c3");
  REQUIRE(fa3->getCompartment()->getId() == "c3");
  REQUIRE(fa3->getId() == "A_c3");
  geometry::Field *fb3 = s.getSpecies().getField("B_c3");
  REQUIRE(fb3->getCompartment()->getId() == "c3");
  REQUIRE(fb3->getId() == "B_c3");

  // check membranes have correct compartment pairs & sizes
  const auto &m0 = s.getMembranes().getMembranes()[0];
  REQUIRE(m0.getId() == "c1_c2_membrane");
  REQUIRE(m0.getCompartmentA()->getId() == "c1");
  REQUIRE(m0.getCompartmentB()->getId() == "c2");
  REQUIRE(m0.getIndexPairs().size() == 1);
  REQUIRE(m0.getIndexPairs()[0] == std::pair<std::size_t, std::size_t>{0, 0});

  const auto &m1 = s.getMembranes().getMembranes()[1];
  REQUIRE(m1.getId() == "c2_c3_membrane");
  REQUIRE(m1.getCompartmentA()->getId() == "c2");
  REQUIRE(m1.getCompartmentB()->getId() == "c3");
  REQUIRE(m1.getIndexPairs().size() == 1);
  REQUIRE(m1.getIndexPairs()[0] == std::pair<std::size_t, std::size_t>{0, 0});

  // move membrane reactions from compartment to membrane
  s.getReactions().setLocation("A_uptake", "c1_c2_membrane");
  s.getReactions().setLocation("A_transport", "c2_c3_membrane");
  s.getReactions().setLocation("B_transport", "c2_c3_membrane");
  s.getReactions().setLocation("B_excretion", "c1_c2_membrane");

  double dt = 0.134521234;
  simulate::Simulation sim(s, simulate::SimulatorType::Pixel);
  auto options = sim.getIntegratorOptions();
  options.order = 1;
  options.maxAbsErr = std::numeric_limits<double>::max();
  options.maxRelErr = std::numeric_limits<double>::max();
  options.maxTimestep = dt;
  sim.setIntegratorOptions(options);
  sim.setMaxThreads(2);
  REQUIRE(sim.getMaxThreads() == 2);

  // check initial concentrations:
  // note: A_c1 is constant, so not part of simulation
  REQUIRE(sim.getAvgMinMax(0, 0, 0).avg == dbl_approx(0.0));
  REQUIRE(sim.getAvgMinMax(0, 1, 0).avg == dbl_approx(0.0));
  REQUIRE(sim.getAvgMinMax(0, 1, 1).avg == dbl_approx(0.0));
  REQUIRE(sim.getAvgMinMax(0, 2, 0).avg == dbl_approx(0.0));
  REQUIRE(sim.getAvgMinMax(0, 2, 1).avg == dbl_approx(0.0));

  // check initial concentration image
  img = sim.getConcImage(0);
  REQUIRE(img.size() == QSize(1, 3));
  REQUIRE(img.pixel(0, 0) == QColor(0, 0, 0).rgba());
  REQUIRE(img.pixel(0, 1) == QColor(0, 0, 0).rgba());
  REQUIRE(img.pixel(0, 2) == QColor(0, 0, 0).rgba());

  double volC1 = 10.0;
  WHEN("single Euler step") {
    auto steps = sim.doTimestep(dt);
    REQUIRE(steps == 1);
    std::size_t it = 1;
    // B_c1 = 0
    REQUIRE(sim.getAvgMinMax(it, 0, 0).avg == dbl_approx(0.0));
    // A_c2 += k_1 A_c1 dt
    REQUIRE(sim.getAvgMinMax(it, 1, 0).avg == dbl_approx(0.1 * 1.0 * dt));
    // B_c2 = 0
    REQUIRE(sim.getAvgMinMax(it, 1, 1).avg == dbl_approx(0.0));
    // A_c3 = 0
    REQUIRE(sim.getAvgMinMax(it, 2, 0).avg == dbl_approx(0.0));
    // B_c3 = 0
    REQUIRE(sim.getAvgMinMax(it, 2, 1).avg == dbl_approx(0.0));
  }

  WHEN("two Euler steps") {
    auto steps = sim.doTimestep(dt);
    REQUIRE(steps == 1);
    double A_c2 = sim.getAvgMinMax(1, 1, 0).avg;
    steps = sim.doTimestep(dt);
    REQUIRE(steps == 1);
    std::size_t it = 2;
    // B_c1 = 0
    REQUIRE(sim.getAvgMinMax(it, 0, 0).avg == dbl_approx(0.0));
    // A_c2 += k_1 A_c1 dt - k1 * A_c2 * dt
    REQUIRE(sim.getAvgMinMax(it, 1, 0).avg ==
            dbl_approx(A_c2 + 0.1 * dt - A_c2 * 0.1 * dt));
    // B_c2 = 0
    REQUIRE(sim.getAvgMinMax(it, 1, 1).avg == dbl_approx(0.0));
    // A_c3 += k_1 A_c2 dt / c3
    REQUIRE(sim.getAvgMinMax(it, 2, 0).avg == dbl_approx(0.1 * A_c2 * dt));
    // B_c3 = 0
    REQUIRE(sim.getAvgMinMax(it, 2, 1).avg == dbl_approx(0.0));
  }

  WHEN("three Euler steps") {
    sim.doTimestep(dt);
    sim.doTimestep(dt);
    double A_c2 = sim.getAvgMinMax(2, 1, 0).avg;
    double A_c3 = sim.getAvgMinMax(2, 2, 0).avg;
    sim.doTimestep(dt);
    std::size_t it = 3;
    // B_c1 = 0
    REQUIRE(sim.getAvgMinMax(it, 0, 0).avg == dbl_approx(0.0));
    // A_c2 += k_1 (A_c1 - A_c2) dt + k2 * A_c3 * dt
    REQUIRE(sim.getAvgMinMax(it, 1, 0).avg ==
            dbl_approx(A_c2 + 0.1 * dt - A_c2 * 0.1 * dt + A_c3 * 0.1 * dt));
    // B_c2 = 0
    REQUIRE(sim.getAvgMinMax(it, 1, 1).avg == dbl_approx(0.0));
    // A_c3 += k_1 A_c2 dt - k_1 A_c3 dt -
    REQUIRE(sim.getAvgMinMax(it, 2, 0).avg ==
            dbl_approx(A_c3 + (0.1 * (A_c2 - A_c3) * dt - 0.3 * A_c3 * dt)));
    // B_c3 = 0.2 * 0.3 * A_c3 * dt
    REQUIRE(sim.getAvgMinMax(it, 2, 1).avg == dbl_approx(0.3 * A_c3 * dt));
  }

  WHEN("many Euler steps -> steady state solution") {
    // when A & B saturate in all compartments, we reach a steady state
    // by conservation: flux of B of into c1 = flux of A from c1 = 0.1
    // all other net fluxes are zero
    double acceptable_error = 1.e-8;
    options.maxTimestep = 0.20138571;
    sim.setIntegratorOptions(options);
    sim.doTimestep(1000);
    std::size_t it = sim.getTimePoints().size() - 1;
    double A_c1 = 1.0;
    double A_c2 = sim.getAvgMinMax(it, 1, 0).avg;
    double A_c3 = sim.getAvgMinMax(it, 2, 0).avg;
    double B_c1 = sim.getAvgMinMax(it, 0, 0).avg;
    double B_c2 = sim.getAvgMinMax(it, 1, 1).avg;
    double B_c3 = sim.getAvgMinMax(it, 2, 1).avg;

    // check concentration values
    REQUIRE(A_c1 == Approx(1.0).epsilon(acceptable_error));
    REQUIRE(
        A_c2 ==
        Approx(0.5 * A_c1 * (0.06 + 0.10) / 0.06).epsilon(acceptable_error));
    REQUIRE(A_c3 == Approx(A_c2 - A_c1).epsilon(acceptable_error));
    // B_c1 "steady state" solution is linear growth
    REQUIRE(B_c3 ==
            Approx((0.06 / 0.10) * A_c3 / 0.2).epsilon(acceptable_error));
    REQUIRE(B_c2 == Approx(B_c3 / 2.0).epsilon(acceptable_error));

    // check concentration derivatives
    double eps = 1.e-5;
    options.maxAbsErr = std::numeric_limits<double>::max();
    options.maxRelErr = std::numeric_limits<double>::max();
    options.maxTimestep = eps;
    sim.setIntegratorOptions(options);
    sim.doTimestep(eps);
    ++it;
    double dA2 = (sim.getAvgMinMax(it, 1, 0).avg - A_c2) / eps;
    REQUIRE(dA2 == Approx(0).epsilon(acceptable_error));
    double dA3 = (sim.getAvgMinMax(it, 2, 0).avg - A_c3) / eps;
    REQUIRE(dA3 == Approx(0).epsilon(acceptable_error));
    double dB1 = volC1 * (sim.getAvgMinMax(it, 0, 0).avg - B_c1) / eps;
    REQUIRE(dB1 == Approx(1).epsilon(acceptable_error));
    double dB2 = (sim.getAvgMinMax(it, 1, 1).avg - B_c2) / eps;
    REQUIRE(dB2 == Approx(0).epsilon(acceptable_error));
    double dB3 = (sim.getAvgMinMax(it, 2, 1).avg - B_c3) / eps;
    REQUIRE(dB3 == Approx(0).epsilon(acceptable_error));
  }
}

SCENARIO("Simulate: very_simple_model, 2d geometry",
         "[core/simulate/simulate][core/simulate][core][simulate][pixel]") {
  // import model
  model::Model s;
  QFile f(":/models/very-simple-model.xml");
  f.open(QIODevice::ReadOnly);
  s.importSBMLString(f.readAll().toStdString());

  // check fields have correct compartments & sizes
  const auto *fa1 = s.getSpecies().getField("A_c1");
  REQUIRE(fa1->getCompartment()->getId() == "c1");
  REQUIRE(fa1->getId() == "A_c1");
  const auto *fb1 = s.getSpecies().getField("B_c1");
  REQUIRE(fb1->getCompartment()->getId() == "c1");
  REQUIRE(fb1->getId() == "B_c1");
  const auto *fa2 = s.getSpecies().getField("A_c2");
  REQUIRE(fa2->getCompartment()->getId() == "c2");
  REQUIRE(fa2->getId() == "A_c2");
  const auto *fb2 = s.getSpecies().getField("B_c2");
  REQUIRE(fb2->getCompartment()->getId() == "c2");
  REQUIRE(fb2->getId() == "B_c2");
  const auto *fa3 = s.getSpecies().getField("A_c3");
  REQUIRE(fa3->getCompartment()->getId() == "c3");
  REQUIRE(fa3->getId() == "A_c3");
  const auto *fb3 = s.getSpecies().getField("B_c3");
  REQUIRE(fb3->getCompartment()->getId() == "c3");
  REQUIRE(fb3->getId() == "B_c3");

  // check membranes have correct compartment pairs & sizes
  const auto &m0 = s.getMembranes().getMembranes()[0];
  REQUIRE(m0.getId() == "c1_c2_membrane");
  REQUIRE(m0.getCompartmentA()->getId() == "c1");
  REQUIRE(m0.getCompartmentB()->getId() == "c2");
  REQUIRE(m0.getIndexPairs().size() == 338);

  const auto &m1 = s.getMembranes().getMembranes()[1];
  REQUIRE(m1.getId() == "c2_c3_membrane");
  REQUIRE(m1.getCompartmentA()->getId() == "c2");
  REQUIRE(m1.getCompartmentB()->getId() == "c3");
  REQUIRE(m1.getIndexPairs().size() == 108);

  simulate::Simulation sim(s, simulate::SimulatorType::Pixel);
  auto options = sim.getIntegratorOptions();
  options.order = 1;
  sim.setIntegratorOptions(options);

  // check initial concentrations:
  // note: A_c1 is constant, so not part of simulation
  REQUIRE(sim.getAvgMinMax(0, 0, 0).avg == dbl_approx(0.0));
  REQUIRE(sim.getAvgMinMax(0, 1, 0).avg == dbl_approx(0.0));
  REQUIRE(sim.getAvgMinMax(0, 1, 1).avg == dbl_approx(0.0));
  REQUIRE(sim.getAvgMinMax(0, 2, 0).avg == dbl_approx(0.0));
  REQUIRE(sim.getAvgMinMax(0, 2, 1).avg == dbl_approx(0.0));

  WHEN("one Euler steps: diffusion of A into c2") {
    options.maxAbsErr = std::numeric_limits<double>::max();
    options.maxRelErr = std::numeric_limits<double>::max();
    options.maxTimestep = 0.01;
    sim.setIntegratorOptions(options);
    sim.doTimestep(0.01);
    REQUIRE(sim.getAvgMinMax(1, 0, 0).avg == dbl_approx(0.0));
    REQUIRE(sim.getAvgMinMax(1, 1, 0).avg > 0);
    REQUIRE(sim.getAvgMinMax(1, 1, 1).avg == dbl_approx(0.0));
    REQUIRE(sim.getAvgMinMax(1, 2, 0).avg == dbl_approx(0.0));
    REQUIRE(sim.getAvgMinMax(1, 2, 1).avg == dbl_approx(0.0));
  }

  WHEN("many Euler steps: all species non-zero") {
    options.maxAbsErr = std::numeric_limits<double>::max();
    options.maxRelErr = std::numeric_limits<double>::max();
    options.maxTimestep = 0.02;
    sim.setIntegratorOptions(options);
    sim.doTimestep(1.00);
    REQUIRE(sim.getAvgMinMax(1, 0, 0).avg > 0);
    REQUIRE(sim.getAvgMinMax(1, 1, 0).avg > 0);
    REQUIRE(sim.getAvgMinMax(1, 1, 1).avg > 0);
    REQUIRE(sim.getAvgMinMax(1, 2, 0).avg > 0);
    REQUIRE(sim.getAvgMinMax(1, 2, 1).avg > 0);
  }
}

// return r2 distance from origin of point
static double r2(const QPoint &p) {
  // nb: flip y axis since qpoint has y=0 at top
  return std::pow(p.x() - 48, 2) + std::pow((99 - p.y()) - 48, 2);
}

// return analytic prediction for concentration
// u(t) = [t0/(t+t0)] * exp(-r^2/(4Dt))
static double analytic(const QPoint &p, double t, double D, double t0) {
  return (t0 / (t + t0)) * exp(-r2(p) / (4.0 * D * (t + t0)));
}

SCENARIO(
    "Simulate: single-compartment-diffusion, circular geometry",
    "[core/simulate/simulate][core/simulate][core][simulate][dune][pixel]") {
  // see docs/tests/diffusion.rst for analytic expressions used here
  // NB central point of initial distribution: (48,99-48) <-> ix=1577

  constexpr double pi = 3.14159265358979323846;
  double sigma2 = 36.0;
  double epsilon = 1e-10;
  // import model
  model::Model s;
  if (QFile f(":/models/single-compartment-diffusion.xml");
      f.open(QIODevice::ReadOnly)) {
    s.importSBMLString(f.readAll().toStdString());
  }

  // check fields have correct compartments
  const auto *slow = s.getSpecies().getField("slow");
  REQUIRE(slow->getCompartment()->getId() == "circle");
  REQUIRE(slow->getId() == "slow");
  const auto *fast = s.getSpecies().getField("fast");
  REQUIRE(fast->getCompartment()->getId() == "circle");
  REQUIRE(fast->getId() == "fast");

  // check total initial concentration matches analytic value
  double analytic_total = sigma2 * pi;
  for (const auto &c : {slow->getConcentration(), fast->getConcentration()}) {
    REQUIRE(std::abs(utils::sum(c) - analytic_total) / analytic_total <
            epsilon);
  }

  // check initial distribution matches analytic one
  for (const auto &f : {slow, fast}) {
    double D = f->getDiffusionConstant();
    double t0 = sigma2 / 4.0 / D;
    double maxRelErr = 0;
    for (std::size_t i = 0; i < f->getCompartment()->nPixels(); ++i) {
      const auto &p = f->getCompartment()->getPixel(i);
      double c = analytic(p, 0, D, t0);
      double relErr = std::abs(f->getConcentration()[i] - c) / c;
      maxRelErr = std::max(maxRelErr, relErr);
    }
    CAPTURE(f->getDiffusionConstant());
    REQUIRE(maxRelErr < epsilon);
  }

  for (auto simType :
       {simulate::SimulatorType::Pixel, simulate::SimulatorType::DUNE}) {
    double initialRelativeError = 1e-9;
    double evolvedRelativeError = 0.02;
    double simRelErr = 0.01;
    double dt = std::numeric_limits<double>::max();
    if (simType == simulate::SimulatorType::DUNE) {
      initialRelativeError = 0.05;
      evolvedRelativeError = 0.5;
      simRelErr = std::numeric_limits<double>::max();
      dt = 1.0;
    }

    // integrate & compare
    simulate::Simulation sim(s, simType);
    double t = 10.0;
    for (std::size_t step = 0; step < 2; ++step) {
      auto options = sim.getIntegratorOptions();
      options.maxAbsErr = std::numeric_limits<double>::max();
      options.maxRelErr = simRelErr;
      options.maxTimestep = dt;
      sim.setIntegratorOptions(options);
      sim.doTimestep(t);
      for (auto speciesIndex : {std::size_t{0}, std::size_t{1}}) {
        // check total concentration is conserved
        auto c = sim.getConc(step + 1, 0, speciesIndex);
        double totalC = utils::sum(c);
        double relErr = std::abs(totalC - analytic_total) / analytic_total;
        CAPTURE(simType);
        CAPTURE(speciesIndex);
        CAPTURE(sim.getTimePoints().back());
        CAPTURE(totalC);
        CAPTURE(analytic_total);
        REQUIRE(relErr < initialRelativeError);
      }
    }

    // check new distribution matches analytic one
    std::vector<double> D{slow->getDiffusionConstant(),
                          fast->getDiffusionConstant()};
    std::size_t timeIndex = sim.getTimePoints().size() - 1;
    t = sim.getTimePoints().back();
    for (auto speciesIndex : {std::size_t{0}, std::size_t{1}}) {
      double t0 = sigma2 / 4.0 / D[speciesIndex];
      auto conc = sim.getConc(timeIndex, 0, speciesIndex);
      double maxRelErr = 0;
      for (std::size_t i = 0; i < slow->getCompartment()->nPixels(); ++i) {
        const auto &p = slow->getCompartment()->getPixel(i);
        // only check part within a radius of 16 units from centre to avoid
        // boundary effects: analytic solution is in infinite volume
        if (r2(p) < 16 * 16) {
          double c_analytic = analytic(p, t, D[speciesIndex], t0);
          double relErr = std::abs(conc[i] - c_analytic) / c_analytic;
          maxRelErr = std::max(maxRelErr, relErr);
        }
      }
      CAPTURE(simType);
      CAPTURE(t);
      REQUIRE(maxRelErr < evolvedRelativeError);
    }
  }
}

SCENARIO(
    "Simulate: small-single-compartment-diffusion, circular geometry",
    "[core/simulate/simulate][core/simulate][core][simulate][dune][pixel]") {
  WHEN("many steps: both species end up equally & uniformly distributed") {
    double epsilon = 1e-3;
    // import model
    model::Model s;
    if (QFile f(":/models/small-single-compartment-diffusion.xml");
        f.open(QIODevice::ReadOnly)) {
      s.importSBMLString(f.readAll().toStdString());
    }
    for (auto simulator :
         {simulate::SimulatorType::DUNE, simulate::SimulatorType::Pixel}) {
      auto sim = simulate::Simulation(s, simulator);
      double simRelErr = 0.001;
      double dt = std::numeric_limits<double>::max();
      if (simulator == simulate::SimulatorType::DUNE) {
        REQUIRE(sim.getMaxThreads() == 0);
        // DuneSim ignores setMaxThreads call: always single threaded
        sim.setMaxThreads(2);
        REQUIRE(sim.getMaxThreads() == 0);
        dt = 0.5;
        simRelErr = std::numeric_limits<double>::max();
      }
      auto options = sim.getIntegratorOptions();
      options.maxAbsErr = std::numeric_limits<double>::max();
      options.maxRelErr = simRelErr;
      options.maxTimestep = dt;
      sim.setIntegratorOptions(options);
      sim.doTimestep(50.0);
      auto timeIndex = sim.getTimePoints().size() - 1;
      // after many steps in a finite volume, diffusion has reached the limiting
      // case of a uniform distribution
      std::size_t speciesIndex = 0;
      for (const auto &species : {"slow", "fast"}) {
        auto conc = sim.getAvgMinMax(timeIndex, 0, speciesIndex++);
        CAPTURE(simulator);
        CAPTURE(species);
        CAPTURE(conc.min);
        CAPTURE(conc.avg);
        CAPTURE(conc.max);
        REQUIRE(std::abs((conc.min - conc.avg) / conc.avg) < epsilon);
        REQUIRE(std::abs((conc.max - conc.avg) / conc.avg) < epsilon);
      }
    }
  }
}

SCENARIO("Pixel simulator: brusselator model, RK2, RK3, RK4",
         "[core/simulate/simulate][core/simulate][core][simulate][pixel]") {
  double eps = 1e-20;
  double time = 30.0;
  double relErr = 0.01;
  // import model
  model::Model s;
  if (QFile f(":/models/brusselator-model.xml"); f.open(QIODevice::ReadOnly)) {
    s.importSBMLString(f.readAll().toStdString());
  }
  // do accurate simulation
  simulate::Simulation sim(s, simulate::SimulatorType::Pixel);
  auto options = sim.getIntegratorOptions();
  options.order = 4;
  options.maxAbsErr = std::numeric_limits<double>::max();
  options.maxRelErr = 1e-6;
  options.maxTimestep = std::numeric_limits<double>::max();
  sim.setIntegratorOptions(options);
  sim.doTimestep(time);
  auto c4_accurate = sim.getConc(sim.getTimePoints().size() - 1, 0, 0);
  // check lower accuracy & different orders are consistent
  for (std::size_t order = 2; order < 5; ++order) {
    double maxRelDiff = 0;
    simulate::Simulation sim2(s, simulate::SimulatorType::Pixel);
    options.order = order;
    options.maxRelErr = relErr;
    sim2.setIntegratorOptions(options);
    sim2.doTimestep(time);
    auto conc = sim2.getConc(sim.getTimePoints().size() - 1, 0, 0);
    for (std::size_t i = 0; i < conc.size(); ++i) {
      maxRelDiff = std::max(
          maxRelDiff, (conc[i] - c4_accurate[i]) / (c4_accurate[i] + eps));
    }
    CAPTURE(order);
    REQUIRE(maxRelDiff < relErr);
  }
}
SCENARIO("DUNE: simulation",
         "[core/simulate/simulate][core/simulate][core][simulate][dune]") {
  GIVEN("ABtoC model") {
    model::Model s;
    if (QFile f(":/models/ABtoC.xml"); f.open(QIODevice::ReadOnly)) {
      s.importSBMLString(f.readAll().toStdString());
    }

    // set spatially constant initial conditions
    s.getSpecies().setInitialConcentration("A", 1.0);
    s.getSpecies().setInitialConcentration("B", 1.0);
    s.getSpecies().setInitialConcentration("C", 0.0);

    simulate::Simulation duneSim(s);
    auto options = duneSim.getIntegratorOptions();
    options.maxTimestep = 0.01;
    duneSim.setIntegratorOptions(options);
    REQUIRE(duneSim.getAvgMinMax(0, 0, 0).avg == dbl_approx(1.0));
    REQUIRE(duneSim.getAvgMinMax(0, 0, 1).avg == dbl_approx(1.0));
    REQUIRE(duneSim.getAvgMinMax(0, 0, 2).avg == dbl_approx(0.0));
    duneSim.doTimestep(0.05);
    auto timeIndex = duneSim.getTimePoints().size() - 1;
    auto imgConc = duneSim.getConcImage(timeIndex);
    REQUIRE(std::abs(duneSim.getAvgMinMax(timeIndex, 0, 0).avg - 0.995) < 1e-4);
    REQUIRE(std::abs(duneSim.getAvgMinMax(timeIndex, 0, 1).avg - 0.995) < 1e-4);
    REQUIRE(std::abs(duneSim.getAvgMinMax(timeIndex, 0, 2).avg - 0.005) < 1e-4);
    REQUIRE(imgConc.size() == QSize(100, 100));
  }
  GIVEN("very-simple-model") {
    model::Model s;
    if (QFile f(":/models/very-simple-model.xml");
        f.open(QIODevice::ReadOnly)) {
      s.importSBMLString(f.readAll().toStdString());
    }
    simulate::Simulation duneSim(s);
    auto options = duneSim.getIntegratorOptions();
    options.maxTimestep = 0.01;
    duneSim.setIntegratorOptions(options);
    duneSim.doTimestep(0.01);
    REQUIRE(duneSim.errorMessage().empty());
  }
  /*
  // too slow in debug mode:
  GIVEN("brusselator-model, too large timestep") {
    sbml::SbmlDocWrapper s;
    if (QFile f(":/models/brusselator-model.xml");
        f.open(QIODevice::ReadOnly)) {
      s.importSBMLString(f.readAll().toStdString());
    }
    for (auto order : {std::size_t{1}, std::size_t{2}}) {
      simulate::Simulation duneSim(s, simulate::SimulatorType::DUNE, order);
      auto options = duneSim.getIntegratorOptions();
      options.maxTimestep = 50.0;
      duneSim.setIntegratorOptions(options);
      // DUNE simulation will fail, but Simulation wrapper should catch
      // exception...
      REQUIRE_NOTHROW(duneSim.doTimestep(500.0));
      // and put the error message here:
      REQUIRE(!duneSim.errorMessage().empty());
    }
  }
  */
}