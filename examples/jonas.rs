use std::path::PathBuf;
use ttf_parser as ttf;

// use graphics::path::PathSegment;
// use graphics::path::PathSegment::{C, L, M, Q, Z};

const FONT_SIZE: f64 = 128.0;
const COLUMNS: u32 = 10;

fn main() {
    let ttf_path = PathBuf::from("Merriweather-Regular.ttf");
    let svg_path = PathBuf::from("Merriweather-Regular.svg");
    let font_data = std::fs::read(ttf_path).unwrap();
    let now = std::time::Instant::now();

    #[allow(unused_mut)]
    let mut face = ttf::Face::parse(&font_data, 0).unwrap();
    let mut family_names = Vec::new();
    for name in face.names() {
        if name.name_id == ttf_parser::name_id::FULL_NAME && name.is_unicode() {
            if let Some(family_name) = name.to_string() {
                let language = name.language();
                family_names.push(format!(
                    "{} ({}, {})",
                    family_name,
                    language.primary_language(),
                    language.region()
                ));
            }
        }
    }
    println!("family_names: {:?}", family_names);
    println!("number_of_glyphs: {}", face.number_of_glyphs());

    let units_per_em = face.units_per_em();
    let scale = FONT_SIZE / units_per_em as f64;
    let cell_size = face.height() as f64 * FONT_SIZE / units_per_em as f64;
    let rows = (face.number_of_glyphs() as f64 / COLUMNS as f64).ceil() as u32;

    let mut svg = xmlwriter::XmlWriter::with_capacity(
        face.number_of_glyphs() as usize * 512,
        xmlwriter::Options::default(),
    );

    svg.start_element("svg");
    svg.write_attribute("xmlns", "http://www.w3.org/2000/svg");
    svg.write_attribute("xmlns:xlink", "http://www.w3.org/1999/xlink");
    svg.write_attribute_fmt(
        "viewBox",
        format_args!(
            "{} {} {} {}",
            0,
            0,
            cell_size * COLUMNS as f64,
            cell_size * rows as f64
        ),
    );

    // draw_grid(face.number_of_glyphs(), cell_size, &mut svg);
    let mut path_buf = String::with_capacity(256);
    let mut row = 0;
    let mut column = 0;

    let mut outbuffer = String::new();

    for id in 0..face.number_of_glyphs() {
        // for id in 0..5 {
        let x = column as f64 * cell_size;
        let y = row as f64 * cell_size;

        draw_number(id, x, y, cell_size, &mut svg);

        draw_glyph(
            x,
            y,
            &face,
            ttf::GlyphId(id),
            cell_size,
            scale,
            &mut svg,
            &mut path_buf,
            &mut outbuffer,
        );

        column += 1;
        if column == COLUMNS {
            column = 0;
            row += 1;
        }
    }

    println!("Elapsed: {}ms", now.elapsed().as_micros() as f64 / 1000.0);

    std::fs::write(svg_path, &svg.end_document()).unwrap();

    std::fs::write("./merriweather_regular/merriweather_Regular.rs", &outbuffer).unwrap();
}

fn draw_glyph(
    x: f64,
    y: f64,
    face: &ttf::Face,
    glyph_id: ttf::GlyphId,
    cell_size: f64,
    scale: f64,
    svg: &mut xmlwriter::XmlWriter,
    path_buf: &mut String,
    outbuffer: &mut String,
) {
    path_buf.clear();
    if !path_buf.is_empty() {
        path_buf.pop(); // remove trailing space
    }

    // let mut v: Vec<PathSegment> = vec![];
    // let mut builder = YBuilder(&mut v);
    // face.outline_glyph(glyph_id, &mut builder);

    let mut builder = Builder(path_buf);
    let bbox = match face.outline_glyph(glyph_id, &mut builder) {
        Some(v) => v,
        None => return,
    };
    // println!("{:?}", builder);

    let bbox_w = (bbox.x_max as f64 - bbox.x_min as f64) * scale;
    let dx = (cell_size - bbox_w) / 2.0;
    let y = y + cell_size + face.descender() as f64 * scale;
    let transform = format!("matrix({} 0 0 {} {} {})", scale, -scale, x + dx, y);

    svg.start_element("path");
    svg.write_attribute("d", path_buf);
    svg.write_attribute("transform", &transform);
    svg.end_element();

    // let mut builder = PathBuilder(path_buf);
    // let bbox = match face.outline_glyph(glyph_id, &mut builder) {
    //     Some(v) => v,
    //     None => return,
    // };

    println!("- glyph_id:{:?}", glyph_id);
    let mut v: Vec<PathSegment> = vec![];
    let mut builder = YBuilder(&mut v);
    face.outline_glyph(glyph_id, &mut builder);

    let str: String = format!(
        "// pub const GEORGIA_{:?} : &'static [PathSegment] = &{:?}; \n\n",
        glyph_id.0, builder.0
    );

    outbuffer.push_str(&str);

    println!("{}", str);

    // std::fs::write(format!("./georgia/georgia-{}.rs", glyph_id.0), &str)
    //     .expect("Unable to write file");

    // let json = serde_json::to_string(builder.0).unwrap();
    // let filename = format!("./georgia/georgia-{}.json", glyph_id.0);
    // println!("- filename:{:?}", filename);
    // std::fs::write(filename, json).unwrap();
}

#[derive(Debug)]
struct Builder<'a>(&'a mut String);

impl ttf::OutlineBuilder for Builder<'_> {
    fn move_to(&mut self, x: f32, y: f32) {
        use std::fmt::Write;
        write!(self.0, "M {} {} ", x, y).unwrap()
    }

    fn line_to(&mut self, x: f32, y: f32) {
        use std::fmt::Write;
        write!(self.0, "L {} {} ", x, y).unwrap()
    }

    fn quad_to(&mut self, x1: f32, y1: f32, x: f32, y: f32) {
        use std::fmt::Write;
        write!(self.0, "Q {} {} {} {} ", x1, y1, x, y).unwrap()
    }

    fn curve_to(&mut self, x1: f32, y1: f32, x2: f32, y2: f32, x: f32, y: f32) {
        use std::fmt::Write;
        write!(self.0, "C {} {} {} {} {} {} ", x1, y1, x2, y2, x, y).unwrap()
    }

    fn close(&mut self) {
        self.0.push_str("Z ")
    }
}

#[derive(Debug)]
struct PathBuilder<'a>(&'a mut String);

impl ttf::OutlineBuilder for PathBuilder<'_> {
    fn move_to(&mut self, x: f32, y: f32) {
        use std::fmt::Write;
        write!(self.0, "M({}, {}), \n", x, y).unwrap()
    }

    fn line_to(&mut self, x: f32, y: f32) {
        use std::fmt::Write;
        write!(self.0, "L({}, {}), \n", x, y).unwrap()
    }

    fn quad_to(&mut self, x1: f32, y1: f32, x: f32, y: f32) {
        use std::fmt::Write;
        write!(self.0, "Q({}, {}, {}, {}), \n", x1, y1, x, y).unwrap()
    }

    fn curve_to(&mut self, x1: f32, y1: f32, x2: f32, y2: f32, x: f32, y: f32) {
        use std::fmt::Write;
        write!(
            self.0,
            "C({}, {}, {}, {}, {}, {}), \n",
            x1, y1, x2, y2, x, y
        )
        .unwrap()
    }

    fn close(&mut self) {
        self.0.push_str("Z, \n")
    }
}

#[derive(Debug)]
struct YBuilder<'a>(&'a mut Vec<PathSegment>);
impl ttf::OutlineBuilder for YBuilder<'_> {
    fn move_to(&mut self, x: f32, y: f32) {
        self.0.push(PathSegment::M(x, y));
    }

    fn line_to(&mut self, x: f32, y: f32) {
        self.0.push(PathSegment::L(x, y));
    }

    fn quad_to(&mut self, x1: f32, y1: f32, x: f32, y: f32) {
        self.0.push(PathSegment::Q(x1, y1, x, y));
    }

    fn curve_to(&mut self, x1: f32, y1: f32, x2: f32, y2: f32, x: f32, y: f32) {
        self.0.push(PathSegment::C(x1, y1, x2, y2, x, y));
    }

    fn close(&mut self) {
        self.0.push(PathSegment::Z);
    }
}

fn draw_number(id: u16, x: f64, y: f64, cell_size: f64, svg: &mut xmlwriter::XmlWriter) {
    svg.start_element("text");
    svg.write_attribute("x", &(x + 2.0));
    svg.write_attribute("y", &(y + cell_size - 4.0));
    svg.write_attribute("font-size", "36");
    svg.write_attribute("fill", "red");
    svg.write_text_fmt(format_args!("{}", &id));
    svg.end_element();
}

fn draw_grid(n_glyphs: u16, cell_size: f64, svg: &mut xmlwriter::XmlWriter) {
    let columns = COLUMNS;
    let rows = (n_glyphs as f64 / columns as f64).ceil() as u32;

    let width = columns as f64 * cell_size;
    let height = rows as f64 * cell_size;

    svg.start_element("path");
    svg.write_attribute("fill", "none");
    svg.write_attribute("stroke", "black");
    svg.write_attribute("stroke-width", "5");

    let mut path = String::with_capacity(256);

    use std::fmt::Write;
    let mut x = 0.0;
    for _ in 0..=columns {
        write!(&mut path, "M {} {} L {} {} ", x, 0.0, x, height).unwrap();
        x += cell_size;
    }

    let mut y = 0.0;
    for _ in 0..=rows {
        write!(&mut path, "M {} {} L {} {} ", 0.0, y, width, y).unwrap();
        y += cell_size;
    }

    path.pop();

    svg.write_attribute("d", &path);
    svg.end_element();
}

use serde::{Deserialize, Serialize};

#[derive(Debug, Serialize, Deserialize)]
enum PathSegment {
    M(f32, f32),
    L(f32, f32),
    Q(f32, f32, f32, f32),
    C(f32, f32, f32, f32, f32, f32),
    Z,
}
